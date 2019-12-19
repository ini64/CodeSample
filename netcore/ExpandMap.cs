

using System;
using System.Collections.Generic;
using AppServer.Controller;
using AppServer.Controller.common;
using CommonLib.Interface;
using CommonLib.Protocol.ExpandMap;
using CoreLib;
using CoreLib.Util;
using FlatBuffers;
using ServerLib.table.Code;
using Table;

namespace AppServer.RemoteDictionary {
  public class ExpandMap : BaseRemoteDictionary {

		public eNetworkResult List(ulong gameUID, out FlatBufferBuilder fbb) {
      fbb = null;

			var collector = new Collector();
			collector.Add(Key.GameUID, gameUID);

			var expandMap = collector.Regist<Controller.ExpandMapFindall>();

			if (expandMap.GetAll() == false) {
				GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.List() ExpandMap.GetAll, gameUID={gameUID}");
				return eNetworkResult.EXPAND_MAP_LIST_NOT_FOUNT_GET_ALL;
			}

      fbb = new FlatBufferBuilder();

			var offsets = new List<Offset<CommonLib.Protocol.ExpandMap.ExpandArea>>();
			foreach (var map in expandMap.Values) {

				var offset = CommonLib.Protocol.ExpandMap.ExpandArea.CreateExpandArea(fbb,
					map.X,
					map.Y,
          map.PriceID,
					map.StartTime.Ticks,
					map.EndTime.Ticks,
					map.IsOpen);
          offsets.Add(offset);
			}

      var vectorOffset = Expand_Map_List_Response.CreateExpandAreasVector(fbb, offsets.ToArray());
      var resOffset = Expand_Map_List_Response.CreateExpand_Map_List_Response(fbb, vectorOffset);
			fbb.Finish(resOffset.Value);

      GameLog.Log(LogLevel.Trace, $"RemoteDictionary.ExpandMap.List(), {gameUID}, {offsets.Count}");

      return eNetworkResult.SUCCESS;
		}

    public eNetworkResult Create(ulong gameUID, short x, short y, out FlatBufferBuilder fbb) {
      fbb = null;

			var collector = new Collector();
			collector.Add(Key.GameUID, gameUID);

      if (x == 0 || y == 0) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create(), Position Is Zero: gameUID={gameUID}, X={x}, Y={y}");
				return eNetworkResult.EXPAND_MAP_CREATE_FAIL_POSITION_IS_ZERO;
      }

      var expandMap = collector.Regist<Controller.ExpandMapFindall>();

			if (expandMap.GetAll() == false) {
				GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() ExpandMap.GetAll, gameUID={gameUID}");
				return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_GET_ALL;
			}

      //같은 위치
      var foundMap = expandMap.Values.Find( (it) => it.X == x && it.Y == y);
      if (foundMap != null) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Duplicate map, gameUID={gameUID}, X={x}, Y={y}");
				return eNetworkResult.EXPAND_MAP_CREATE_DULICATE;
      }

      //인근 위치
      var nearMap = expandMap.Values.Find( (it) => it.X == x || it.Y == y);
      if (nearMap == null) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Near Area Not Found, gameUID={gameUID}, X={x}, Y={y}");
				return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_NEAR_AREA;
      }

      //레벨 체크
      var accountController = collector.Regist<Controller.Account>(); ;
      if (accountController.Get() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Account.Get() Fail, gameUID={gameUID}");
        return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_ACCOUNT;;
      }

      var stoneLevel = tb_MapAreaAdditional.GetStoneLevel(accountController.Level);

      var area = tb_MapAreaAdditional.GetArea(x, y);
      if (area == null) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Not Found Area Info, gameUID={gameUID}, X={x}, Y={y}");
        return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_AREA_INFO;
      }

      if (stoneLevel < area.Stone_Open) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Stone Level is Low, gameUID={gameUID}, StoneLevel={stoneLevel}, AreaLevel={area.Stone_Open}");
        return eNetworkResult.EXPAND_MAP_CREATE_STONE_LEVEL_IS_LOW;;
      }


      //건설갯수 업데이트
      var accountExpandMapController = collector.Regist<AccountExpandMapCount>();
      if (accountExpandMapController.Get() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() accountExpandMapController.Get() Fail : gameUID={gameUID}");
        return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_MAP_COUNT;
      }

      ushort priceID = (ushort)(accountExpandMapController.ExpandMapCount + 1);
      accountExpandMapController.Set(priceID);

      if(!tb_Map_Price.map.ContainsKey(priceID)) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() not found price info : gameUID={gameUID}, count={priceID}");
        return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_PRICE_INFO;
      }
      var priceInfo = tb_Map_Price.map[priceID];

      //생명력 차감
      var assetController = collector.Regist<AccountAsset>();
      if (assetController.Get() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() AccountAsset.Get() Fail : gameUID={gameUID}");
        return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_ASSSET;
      }

      if (assetController.SubLife(priceInfo.InputLife_Count) == false) {
        GameLog.Log(LogLevel.Info, $"RemoteDictionary.ExpandMap.Create() AccountGoods.SubLife() Fail : gameUID={gameUID}, Have={assetController.Life}, Add={priceInfo.InputLife_Count}");
        return eNetworkResult.EXPAND_MAP_CREATE_FAIL_SUB_LIFE;
      }

      //아이템 차감
      Dictionary<short, short> needItems = new Dictionary<short, short>();
      Dictionary<short, short> updatedItems = new Dictionary<short, short>();

      if (priceInfo.Input01_ItemID != 0) {
        needItems[(short)priceInfo.Input01_ItemID] = (short)priceInfo.Input01_Count;
      }
      if (priceInfo.Input02_ItemID != 0) {
        needItems[(short)priceInfo.Input02_ItemID] = (short)priceInfo.Input02_Count;
      }
      if (priceInfo.Input03_ItemID != 0) {
        needItems[(short)priceInfo.Input03_ItemID] = (short)priceInfo.Input03_Count;
      }

      if (needItems.Count > 0) {
        var itemsController = collector.Regist<Items>();
        if (itemsController.GetCount() == false) {
          GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Items.GetCount() Fail, gameUID={gameUID}");
          return eNetworkResult.EXPAND_MAP_CREATE_NOT_FOUND_ITEM;
        }

        foreach (var item in needItems) {
          if (itemsController.Sub(item.Key, item.Value) == false) {
            GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Create() Items.Sub() Fail, gameUID={gameUID}, itemID={item.Key}, count={item.Value}");
            return eNetworkResult.EXPAND_MAP_CREATE_FAIL_SUB_ITEM;
          }
          updatedItems[item.Key] = itemsController.Values[item.Key];
        }
      }

			var expandMapCreater = collector.Regist<Controller.ExpandMapCreate>();
      var now = ServerTime.Now;
      var startTime = now;
      var endTime = startTime.AddSeconds(priceInfo.AreaExtend_Time);
      expandMapCreater.Create(x, y, startTime, priceID, 0);

      if (collector.SetAll() == false) {
        return eNetworkResult.EXPAND_MAP_CREATE_FAIL_SET_ALL;
      }
      fbb = new FlatBufferBuilder();

      var itemOffsets = new List<Offset<CommonLib.Protocol.ExpandMap.Item>>();
      foreach (var item in updatedItems) {
        itemOffsets.Add(CommonLib.Protocol.ExpandMap.Item.CreateItem(fbb, item.Key, (short)item.Value));
      }
      var itemOffsetVector = Create_Expand_Map_Response.CreateUpdatedItemsVector(fbb, itemOffsets.ToArray());

      var resOffset = Create_Expand_Map_Response.CreateCreate_Expand_Map_Response(fbb, x, y, priceID, startTime.Ticks, endTime.Ticks, assetController.Life, itemOffsetVector, priceID);
			fbb.Finish(resOffset.Value);

      return eNetworkResult.SUCCESS;
		}

    public eNetworkResult Update(ulong gameUID, short x, short y, int priceID, DateTime startTime, byte isOpen, out FlatBufferBuilder fbb) {
      fbb = null;

			var collector = new Collector();
			collector.Add(Key.GameUID, gameUID);


      if(!tb_Map_Price.map.ContainsKey(priceID)) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Update() not found price info : gameUID={gameUID}, count={priceID}");
        return eNetworkResult.EXPAND_MAP_UPDATE_NOT_FOUND_PRICE_ID;
      }

      var priceInfo = tb_Map_Price.map[priceID];
      var endTime = startTime.AddSeconds(priceInfo.AreaExtend_Time);

			var expandMap = collector.Regist<Controller.ExpandMapUpdate>();
      expandMap.Update(x, y, priceID, startTime, isOpen);

      if (collector.SetAll() == false) {
        return eNetworkResult.EXPAND_MAP_UPDATE_FAIL_SET_ALL;
      }

      fbb = new FlatBufferBuilder();

      var resOffset = Update_Expand_Map_Response.CreateUpdate_Expand_Map_Response(fbb, x, y, priceID, startTime.Ticks, endTime.Ticks, isOpen);
			fbb.Finish(resOffset.Value);

      return eNetworkResult.SUCCESS;
		}

    public eNetworkResult Open(ulong gameUID, short x, short y, out FlatBufferBuilder fbb) {
      fbb = null;

			var collector = new Collector();
			collector.Add(Key.GameUID, gameUID);

      var findController = collector.Regist<Controller.ExpandMapFind>();
			if (findController.Get(x,y) == false) {
				GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Open() ExpandMapFind.Get() Fail, gameUID={gameUID}, x={x}, y={y}");
				return eNetworkResult.EXPAND_MAP_OPEN_NOT_FOUND_MAP;
			}

			var map = findController.Value;

			// 건설이 완료된 맵인지
			if( map.EndTime > ServerTime.Now) {
				GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Open() cannt open Fail, gameUID={gameUID}, x={x}, y={y}, " +
					$"EndTime={map.EndTime} > ServerTime={ServerTime.Now}");
				return eNetworkResult.EXPAND_MAP_OPEN_INVALID_END_TIME;
			}

      if(!tb_Map_Price.map.ContainsKey(map.PriceID)) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Open() not found price info : gameUID={gameUID}, count={map.PriceID}");
        return eNetworkResult.EXPAND_MAP_OPEN_NOT_FOUND_PRICE_INFO;
      }
      var priceInfo = tb_Map_Price.map[map.PriceID];

      var levelupController = collector.Regist<AccountLevelup>();
      if (levelupController.Get() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.TetraQuest.Open() - AccountLevelup.Get() Fail : GameUID={gameUID}");
        return eNetworkResult.EXPAND_MAP_OPEN_NOT_FOUND_LEVEL_UP;
      }

      if (levelupController.AddExp(priceInfo.Acquire_XP) == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.TetraQuest.Open() - AccountLevelup.AddExp() Fail : GameUID={gameUID}, addExp={priceInfo.Acquire_XP}");
        return eNetworkResult.EXPAND_MAP_OPEN_FAIL_ADD_EXP;
      }

      var openController = collector.Regist<Controller.ExpandMapOpen>();
      openController.Open(x, y);

      if (collector.SetAll() == false) {
        return eNetworkResult.EXPAND_MAP_OPEN_FAIL_SET_ALL;
      }
      fbb = new FlatBufferBuilder();

      var resOffset = Open_Expand_Map_Response.CreateOpen_Expand_Map_Response(fbb, map.X, map.Y, priceInfo.Acquire_XP, levelupController.Value.exp, levelupController.Value.level);
			fbb.Finish(resOffset.Value);

      return eNetworkResult.SUCCESS;
		}

    public eNetworkResult Complete(ulong gameUID, short x, short y, out FlatBufferBuilder fbb) {
      fbb = null;

			var collector = new Collector();
			collector.Add(Key.GameUID, gameUID);

      var findController = collector.Regist<Controller.ExpandMapFind>();
			if (findController.Get(x,y) == false) {
				GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Complete() ExpandMapFind.Get() Fail, gameUID={gameUID}, x={x}, y={y}");
				return eNetworkResult.EXPAND_MAP_COMPLETE_NOT_FOUND_MAP;
			}

			var map = findController.Value;
      if (map.IsOpen == 1) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Complete() Opened Before, gameUID={gameUID}, x={x}, y={y}");
				return eNetworkResult.EXPAND_MAP_COMPLETE_OPENED_BEFORE;
      }

      var tb_constant = default(tb_Constant);
      if (tb_Constant.map.TryGetValue(eConstantType.Immediately_Time_AreaExtend, out tb_constant) == false) {
        GameLog.Log(LogLevel.Error, "RemoteDictionary.ExpandMaps.Complete() not found Immediately_Time_AreaExtend : gameUID={0}", gameUID);
        return eNetworkResult.EXPAND_MAP_COMPLETE_NOT_FOUND_TIME_DATA;
      }
      int baseTime = tb_constant.Value;

      tb_constant = default(tb_Constant);
      if (tb_Constant.map.TryGetValue(eConstantType.Immediately_AbyssCash_AreaExtend, out tb_constant) == false) {
        GameLog.Log(LogLevel.Error, "RemoteDictionary.ExpandMaps.Complete() not found Immediately_AbyssCash_AreaExtend : gameUID={0}", gameUID);
        return eNetworkResult.EXPAND_MAP_COMPLETE_NOT_FOUND_CASH_DATA;
      }
      int baseCash = tb_constant.Value;

      var cash = ServerTime.TimeToCash(map.EndTime, baseTime, baseCash);

      var assetController = collector.Regist<AccountAsset>();
      if (assetController.Get() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Complete() AccountAsset.Get() Fail : gameUID={gameUID}");
        return eNetworkResult.EXPAND_MAP_COMPLETE_NOT_FOUND_ASSET;
      }

      if (assetController.SubCash(cash) == false) {
        GameLog.Log(LogLevel.Info, $"RemoteDictionary.ExpandMap.Complete() AccountGoods.SubCash() Fail : gameUID={gameUID}, Have={assetController.AbyssCash}, Sub={cash}");
        return eNetworkResult.EXPAND_MAP_COMPLETE_FAIL_SUB_CASH;
      }

      if(!tb_Map_Price.map.ContainsKey(map.PriceID)) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Open() not found price info : gameUID={gameUID}, count={map.PriceID}");
        return eNetworkResult.EXPAND_MAP_COMPLETE_NOT_FOUND_PRICE_INFO;
      }
      var priceInfo = tb_Map_Price.map[map.PriceID];

      var levelupController = collector.Regist<AccountLevelup>();
      if (levelupController.Get() == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.ExpandMap.Open() - AccountLevelup.Get() Fail : GameUID={gameUID}");
        return eNetworkResult.EXPAND_MAP_COMPLETE_NOT_FOUND_LEVEL_UP;
      }

      if (levelupController.AddExp(priceInfo.Acquire_XP) == false) {
        GameLog.Log(LogLevel.Error, $"RemoteDictionary.TetraQuest.Open() - AccountLevelup.AddExp() Fail : GameUID={gameUID}, addExp={priceInfo.Acquire_XP}");
        return eNetworkResult.EXPAND_MAP_COMPLETE_FAIL_ADD_EXP;
      }

      var expandMap = collector.Regist<Controller.ExpandMapUpdate>();
      var newStartTime = map.StartTime.Subtract(TimeSpan.FromSeconds(priceInfo.AreaExtend_Time));
      var newEndTime = newStartTime.Add(TimeSpan.FromSeconds(priceInfo.AreaExtend_Time));
      expandMap.Update(x, y, map.PriceID, newStartTime, 0);

      if (collector.SetAll() == false) {
        return eNetworkResult.EXPAND_MAP_COMPLETE_FAIL_SET_ALL;
      }
      fbb = new FlatBufferBuilder();

      var resOffset = Complete_Expand_Map_Response.CreateComplete_Expand_Map_Response(fbb, map.X, map.Y, assetController.AbyssCash, priceInfo.Acquire_XP, 
        levelupController.Value.exp, newStartTime.Ticks, newEndTime.Ticks, levelupController.Value.level);
			fbb.Finish(resOffset.Value);

      return eNetworkResult.SUCCESS;
    }
	}// class ExpandMap
}
