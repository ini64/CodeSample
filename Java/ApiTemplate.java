
/**
 * @author sid
 *
 * @param <I>
 * @param <O>
 */
public abstract class ApiTemplate<I, O> implements ApiRequest {
	protected static Gson gsonChecker;
	protected static Gson gson;
	protected final static BasicBSONEncoder bsonEncoder = new BasicBSONEncoder();
	protected final static BasicBSONDecoder bsonDecoder = new BasicBSONDecoder();

	protected static MongoDBHelperUser dbHelperUser = null;
	protected static MongoDBHelperAccout dbHelperAccount = null;

	protected static Logger logger = null;
	protected static Logger errorLogger = null;

	protected Class<I> inputType = null;
	protected Class<O> outputType = null;

	protected static ApplicationContext springContext;

	protected static final DateFormat ISO8601 = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSZ");

	@SuppressWarnings("static-access")
	@Autowired
	public void init(ApplicationContext springContext) {
		this.springContext = springContext;
		dbHelperUser = springContext.getBean(MongoDBHelperUser.class);
		dbHelperAccount = springContext.getBean(MongoDBHelperAccout.class);
	}

	/**
	 * @param itype
	 * @param otype
	 * GSon용 기본 인터페이스 생성
	 */
	public ApiTemplate(Class<I> itype, Class<O> otype) {
		logger = LogManager.getLogger("packet");
		errorLogger = LogManager.getLogger("error");

		this.inputType = itype;
		this.outputType = otype;

		gsonChecker = new GsonBuilder().registerTypeAdapter(itype, new JsonAnnotatedDeserializer<I>()).create();

		gson = new GsonBuilder().addSerializationExclusionStrategy(new ExclusionStrategy() {
			@Override
			public boolean shouldSkipField(FieldAttributes fieldAttributes) {
				final Expose expose = fieldAttributes.getAnnotation(Expose.class);
				return expose != null && !expose.serialize();
			}

			@Override
			public boolean shouldSkipClass(Class<?> aClass) {
				return false;
			}
		}).addDeserializationExclusionStrategy(new ExclusionStrategy() {
			@Override
			public boolean shouldSkipField(FieldAttributes fieldAttributes) {
				final Expose expose = fieldAttributes.getAnnotation(Expose.class);
				return expose != null && !expose.deserialize();
			}

			@Override
			public boolean shouldSkipClass(Class<?> aClass) {
				return false;
			}
		}).create();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see im3.tp.service.Template.ApiRequest#Json() 
	 * Json 기반 데이터 처리
	 */
	public ByteBuf Json(ByteBuf content) {

		I input = null;

		try {
			input = MakeInputJson(content);
			InputValidation(input);

			O output = Service(input);
			if (output != null) {
				WriteLogData(input);
				return MakeOutputJson(input, output);
			}

		} catch (EscapeServiceException e) {
			logger.error(e.getMessage(), e);
			errorLogger.error(e.getMessage(), e);
			return MakeExceptionJsonOutput(input, e.GetErrorEnum(), e.getMessage());
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			errorLogger.error(e.getMessage(), e);
			return MakeExceptionJsonOutput(input, ErrorEnum.UNDEFINED, e.getMessage());
		}
		return null;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see im3.tp.service.Template.ApiRequest#BSon() 
	 * BSon기반 데이터 처리
	 */
	public ByteBuf BSon(ByteBuf content) {

		I input = null;

		try {
			input = MakeInputBSon(content);
			InputValidation(input);

			O output = Service(input);
			if (output != null) {
				WriteLogData(input);
				return MakeOutputBSon(input, output);
			}

		} catch (EscapeServiceException e) {
			logger.error(e.getMessage(), e);
			errorLogger.error(e.getMessage(), e);
			return MakeExceptionBSonOutput(input, e.GetErrorEnum(), e.getMessage());
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			errorLogger.error(e.getMessage(), e);
			return MakeExceptionBSonOutput(input, ErrorEnum.UNDEFINED, e.getMessage());
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see im3.escape.service.Template.ApiRequest#JsonTest(io.netty.buffer.ByteBuf)
	 * JUnit 지원용 함수
	 */
	public ByteBuf JsonTest(ByteBuf content) {

		I input = null;
		try {
			input = MakeInputJson(content);
			InputValidation(input);

			O output = Service(input);
			if (output != null) {
				return MakeOutputJson(input, output);
			}
		} catch (EscapeServiceException e) {
			logger.error(e.getMessage(), e);
			errorLogger.error(e.getMessage(), e);
			return MakeExceptionJsonOutput(input, e.GetErrorEnum(), e.getMessage());
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			errorLogger.error(e.getMessage(), e);
			return MakeExceptionJsonOutput(input, ErrorEnum.UNDEFINED, e.getMessage());
		}
		return null;
	}

	/**
	 * Input 로그 작업
	 */
	protected void InputLog(String log) {
		logger.info(String.format("%s:%s", inputType.getSimpleName(), log));
	}

	/**
	 * Output 로그 작업
	 */
	protected void OutputLog(String log) {
		logger.info(String.format("%s:%s", outputType.getSimpleName(), log));
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see im3.tp.service.Template.ApiRequest#MakeInputJson() Input에서 텍스트를 뽑아
	 * Gson을 이용해 클래스 오브젝트로 변환
	 */
	public I MakeInputJson(ByteBuf content) {

		String inputText = content.toString(CharsetUtil.UTF_8);
		inputText = inputText.replace("\n", "").replace("\r", "").trim();

		I input = gsonChecker.fromJson(inputText, inputType);

		InputLog(inputText);

		return input;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see im3.tp.service.Template.ApiRequest#MakeInputMsgPack() Input에서 바이트를
	 * 뽑아 MsgPack을 이용해 클래스 오브젝트로 변환
	 */
	public I MakeInputBSon(ByteBuf content) {

		byte[] bytes = new byte[content.readableBytes()];
		content.readBytes(bytes);

		String inputText = bsonDecoder.readObject(bytes).toString();

		I input = gsonChecker.fromJson(inputText, inputType);

		InputLog(inputText);

		return input;
	}

	/**
	 * @param input
	 * @param output
	 * @return 
	 * 출력 값을 Json으로 변환
	 */
	public abstract ByteBuf MakeOutputJson(I input, O output);

	/**
	 * @param input
	 * @param output
	 * @return
	 * 출력값을 BSon으로 변환
	 */
	public abstract ByteBuf MakeOutputBSon(I input, O output);

	
	/**
	 * @param input
	 * @return
	 * @throws EscapeServiceException
	 * 실제 호출 함수
	 */
	public O Service(I input) throws EscapeServiceException {
		return null;
	}

	/**
	 * @param input
	 * @throws BeansException
	 * @throws InterruptedException
	 * 로그 남기는 함수
	 */
	public void WriteLogData(I input) throws BeansException, InterruptedException {
	}

	/**
	 * @param input
	 * @throws EscapeServiceException
	 * 인풋 값 체크
	 */
	public void InputValidation(I input) throws EscapeServiceException {

	}

	/**
	 * @param input
	 * @param errorEnum
	 * @param info
	 * @return
	 * 문제 발생 시 처리
	 */
	protected abstract ByteBuf MakeExceptionJsonOutput(I input, ErrorEnum errorEnum, String info);

	/**
	 * @param input
	 * @param errorEnum
	 * @param info
	 * @return
	 * 문제 발생 시 처리
	 */
	protected abstract ByteBuf MakeExceptionBSonOutput(I input, ErrorEnum errorEnum, String info);

}
