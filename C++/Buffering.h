
    class Buffering
        : public Singleton2<Buffering, ThreadLocal>
    {
        typedef Mem::FixedMEM<Int8,  32>     B8Pool;
        typedef Mem::FixedMEM<Int16, 32>    B16Pool;
        typedef Mem::FixedMEM<Int32, 32>    B32Pool;
        typedef Mem::FixedMEM<Int64, 32>    B64Pool;

        typedef UInt8 __BLOB[BIGBUF];

        typedef Mem::FixedMEM<__BLOB,  32>    BVAPool;

    public:
        void* Get8()  { return b8.Alloc(); }
        void* Get16() { return b16.Alloc(); }
        void* Get32() { return b32.Alloc(); }
        void* Get64() { return b64.Alloc(); }

        // for variable-length data.
        void* GetVA() { return bVA.Alloc(); }

        enum { VA_LENGTH = sizeof(__BLOB), };
        enum { TS_LENGTH = sizeof(TIMESTAMP_STRUCT), };

    public:
        enum eSizeType
        {
            ST_8,
            ST_16,
            ST_32,
            ST_64,
            ST_VA,
        };

        void Release(void* ptr, eSizeType sizeType)
        {
            switch(sizeType)
            {
                case ST_8:  b8.Free( (Int8*)ptr); break;
                case ST_16: b16.Free((Int16*)ptr); break;
                case ST_32: b32.Free((Int32*)ptr); break;
                case ST_64: b64.Free((Int64*)ptr); break;
                case ST_VA: bVA.Free( (__BLOB*)ptr); break;

                default:
                    NoDefault;
            }
        }

    private:
        B8Pool  b8;
        B16Pool b16;
        B32Pool b32;
        B64Pool b64;
        BVAPool bVA;
};

