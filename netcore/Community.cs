using System.Collections.Generic;
using System.Threading.Tasks;
using AppServer.Controller.common;
using CommonLib.Interface;
using CoreLib;
using CoreLib.Cache;
using Google.Protobuf;
using MessagePack;
using ServerLib.DBData;
using StackExchange.Redis;

namespace AppServer.RemoteDictionary
{
  public class Community : BaseRemoteDictionary {

    const long MaxRecommendPoolSize = 2;
		private IDatabase Redis { get; set; }

		public Community() {
		  Redis = CacheRepository.GetRedis(DBCacheType.GAME_DATA_RECOMMEND).DB;
		}

    /// <summary>
    /// 추천친구 리스트
    /// </summary>
    /// <param name="gameUID"></param>
    /// <param name="proto"></param>
    /// <returns></returns>
		public async Task<(eNetworkResult, CommonLib.Protocol.Recommend_Friend_List_Response)> RecommendFriendListAsync(uint gameUID) {
      CommonLib.Protocol.Recommend_Friend_List_Response proto = new CommonLib.Protocol.Recommend_Friend_List_Response();


      long resultSize = 12;
      long resultHalf = resultSize / 2;
      //List<CommonLib.Protocol.Friend> friends = new List<CommonLib.Protocol.Friend>();

      var collector = new Collector();
			collector.Add(Key.GameUID, gameUID);

      var assetController = collector.Regist<Controller.AccountAsset>();
      if (await assetController.GetAsync() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.Community.RecommendFriend() AccountAsset.Get() Fail, gameUID={gameUID}");
        return (eNetworkResult.RECOMMEND_FRIEND_FAIL_TO_GET_ASSET, proto);
      }

			var levelKey = CoreLib.Redis.Key.RecommendFriendLevel();
      long endPos = await Redis.SortedSetLengthAsync(levelKey);

      //한건도 없는 경우 체크 
      if (endPos >= 0) {
        long pos = endPos;

        var uidArray = await Redis.SortedSetRangeByScoreAsync(levelKey, assetController.Level, assetController.Level, take:1, order:Order.Descending);
        
        int range = 1;
        while (uidArray.Length == 0) {
          if (range > 10) {
            break;
          }
          uidArray = await Redis.SortedSetRangeByScoreAsync(levelKey, assetController.Level - range, assetController.Level + range, take: 1, order: Order.Descending);
          range++;
        }

        if (uidArray.Length > 0) {
          var rank = await Redis.SortedSetRankAsync(levelKey, uidArray[0]);
          if (rank.HasValue)
          {
            pos = rank.Value;
          }
        }

        var start = pos - resultHalf;
        var end = pos + resultHalf;

        //오버 체크
        if (end > endPos) {
          var length = end - endPos;
          //뒤에 인원이 부족하면 앞으로 
          start -= length;
        }

        if (start < 0) {
          start = 0;
        }

        //최종 조율
        end = start + resultSize;
        if (end > endPos) {
          end = endPos;
        }

        var uids = await Redis.SortedSetRangeByRankAsync(levelKey, start, end);
        foreach(var uid in uids) {
          //todo 나중에 친구 관계도 체크 해야함
          if ((ulong)uid == gameUID) continue;
          if (proto.Friends.Count == 10) break;

          var cacheKey = CoreLib.Redis.Key.RecommendFriendCache((uint)uid);
          //비동기라 꼼꼼하게 체크해야 함
          if (await Redis.KeyExistsAsync(cacheKey)){
            var cache = await Redis.StringGetAsync(cacheKey);
            //비동기라 꼼꼼하게 체크해야 함
            if (cache.HasValue) {
              var friend = MessagePackSerializer.Deserialize<db_friend>(cache);

              var packetFriend = new CommonLib.Protocol.Friend() {
                GameUID = friend.gameUID,
                ProfileID = friend.profileID,
                WorldName = friend.worldName,
                Level = friend.level,
              };

              proto.Friends.Add(packetFriend);
            }
          }
        }
      }

      return (eNetworkResult.SUCCESS, proto);
		}

    public async Task<(eNetworkResult, CommonLib.Protocol.Recommend_Friend_Info_Response)> RecommendFriendInfoAsync(uint gameUID, uint targetGameUID)
    {
      CommonLib.Protocol.Recommend_Friend_Info_Response proto = new CommonLib.Protocol.Recommend_Friend_Info_Response()
      {
        Account = new CommonLib.Protocol.Friend(),
      };
     
      proto.Account.GameUID = 0;

      var cacheKey = CoreLib.Redis.Key.RecommendFriendCache(targetGameUID);
      //비동기라 꼼꼼하게 체크해야 함
      if (await Redis.KeyExistsAsync(cacheKey))
      {
        var cache = await Redis.StringGetAsync(cacheKey);
        //비동기라 꼼꼼하게 체크해야 함
        if (cache.HasValue)
        {
          var friend = MessagePackSerializer.Deserialize<db_friend>(cache);

          proto.Account.GameUID = friend.gameUID;
          proto.Account.ProfileID = friend.profileID;
          proto.Account.WorldName = friend.worldName;
          proto.Account.Level = friend.level;

          foreach (var dbBuilding in friend.buildings) {
            proto.Account.Buildings.Add( new CommonLib.Protocol.Building2(){
              BuildingUID = dbBuilding.BuildingUID,
              BuildingID  = dbBuilding.BuildingID,
              PosX  = dbBuilding.PosX,
              PosY  = dbBuilding.PosY,
              PosZ = dbBuilding.PosZ,
              Rot = dbBuilding.Rot,
              Level = dbBuilding.Level,
              Status = (CommonLib.Protocol.Building2.Types.eStatus)dbBuilding.Status,
              SkinID = dbBuilding.SkinID,
            });
          }

          foreach (var dbFish in friend.fishes) {
            proto.Account.Fishes.Add( new CommonLib.Protocol.Fish() {
              FishUID = dbFish.FishUID,
              FishID = dbFish.FishID,
              Status = (CommonLib.Protocol.Fish.Types.eStatus)dbFish.Status,
              AcquireTime = dbFish.AcquireTime.Ticks,
              Size = dbFish.Size,
              GrowthLevel = dbFish.GrowthLevel,
              GrowthExp = dbFish.GrowthExp,
              GrowthTime = dbFish.GrowthTime.Ticks
            });
          }
        }
      }

      //캐시에서 찾을 수 없음
      if (proto.Account.GameUID == 0) {
        var collector = new Collector();
        collector.Add(Key.GameUID, targetGameUID);

        var assetController = collector.Regist<Controller.AccountAsset>();
        if (await assetController.GetAsync() == false)
        {
          GameLog.Log(LogLevel.Error, $"RemoteDictionary.Community.RecommendFriendInfo() AccountAsset.Get() Fail, gameUID={gameUID}");
          return (eNetworkResult.RECOMMEND_FRIEND_INFO_FAIL_TO_GET_ASSET, proto);
        }

        proto.Account.GameUID = targetGameUID;
        proto.Account.ProfileID = assetController.ProfileID;
        proto.Account.WorldName = assetController.WorldName;
        proto.Account.Level = assetController.Level;

        var buildingsController = collector.Regist<Controller.BuildingFindall>();

        if (await buildingsController.GetAsync() == false)
        {
          GameLog.Log(LogLevel.Error, $"RemoteDictionary.Community.RecommendFriendInfo() BuildingFindall.Get, gameUID={gameUID}");
          return (eNetworkResult.RECOMMEND_FRIEND_INFO_FAIL_TO_LOAD_GETBUILDINGS, proto);
        }

        foreach (var dbBuilding in buildingsController.DBValues)
        {
          proto.Account.Buildings.Add(new CommonLib.Protocol.Building2()
          {
            BuildingUID = dbBuilding.BuildingUID,
            BuildingID = dbBuilding.BuildingID,
            PosX = dbBuilding.PosX,
            PosY = dbBuilding.PosY,
            PosZ = dbBuilding.PosZ,
            Rot = dbBuilding.Rot,
            Level = dbBuilding.Level,
            Status = (CommonLib.Protocol.Building2.Types.eStatus)dbBuilding.Status,
            SkinID = dbBuilding.SkinID,
          });
        }

        var fishesController = collector.Regist<Controller.FishesFindall>();

        if (await fishesController.GetAsync() == false)
        {
          GameLog.Log(LogLevel.Error, $"RemoteDictionary.Community.RecommendFriendInfo() Fishes.Get, gameUID={gameUID}");
          return (eNetworkResult.RECOMMEND_FRIEND_INFO_FAIL_TO_LOAD_FISHES, proto);
        }

        foreach (var dbFish in fishesController.Values)
        {
          proto.Account.Fishes.Add(new CommonLib.Protocol.Fish()
          {
            FishUID = dbFish.FishUID,
            FishID = dbFish.FishID,
            Status = (CommonLib.Protocol.Fish.Types.eStatus)dbFish.Status,
            AcquireTime = dbFish.AcquireTime.Ticks,
            Size = dbFish.Size,
            GrowthLevel = dbFish.GrowthLevel,
            GrowthExp = dbFish.GrowthExp,
            GrowthTime = dbFish.GrowthTime.Ticks
          });
        }

      }

      return (eNetworkResult.SUCCESS, proto);
    }





  }// class Community
}
