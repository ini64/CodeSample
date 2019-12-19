using CommonLib.Protocol;
using HBJungTest;
using System.Collections.Generic;
using Test;
using Xunit;


namespace Landmark
{

  public class BasicTest : Tester {

    public BasicTest(Session session, Tables tables) : base(session, tables) {
    }

    [Fact, TestPriority(1)]
    public void T01List() {
      eNetworkResult checkRst = eNetworkResult.SUCCESS;

      Reset();
      Login((rst, obt) => {
        checkRst = rst;
        Move();
      });

      Wait();

      Assert.Equal(eNetworkResult.SUCCESS, checkRst);

      Reset();
      HttpNetMgr.RequestLandmarkInfo((rst, obt) => {
        checkRst = rst;
        Move();
      });

      Wait();

      Assert.Equal(eNetworkResult.SUCCESS, checkRst);
    }

    [Fact, TestPriority(2)]
    public void T02Reward() {
      eNetworkResult checkRst = eNetworkResult.SUCCESS;

      Reset();
      Login((rst, obt) => {
        checkRst = rst;
        Move();
      });

      Wait();

      Assert.Equal(eNetworkResult.SUCCESS, checkRst);

      Reset();
      HttpNetMgr.RequestAsset(999999, 999999, 999999, 999999, 999999, resultCallback: (rst, obt) => {
        checkRst = rst;
        Move();
      });
      Wait();

      Assert.Equal(eNetworkResult.SUCCESS, checkRst);

      Reset();
      HttpNetMgr.RequestAddItem(7, 999, resultCallback: (rst, obt) => {
        checkRst = rst;
        Move();
      });
      Wait();

      Assert.Equal(eNetworkResult.SUCCESS, checkRst);

      for (int i = 2; i < 10; i++) {

        Reset();
        HttpNetMgr.RequestOpenArea((byte)i, (rst, obt) => {
          checkRst = rst;
          var response = obt as Building_Create_Response;
          Move();
        });
        Wait();

        Assert.Equal(checkRst, checkRst);
      }

      int buildingUID1 = 0;
      int buildingUID2 = 0;
      int buildingUID3 = 0;
      Reset();
      HttpNetMgr.RequestBuildingList((rst, obt) =>
      {
        checkRst = rst;
        Building_List_Response res = obt as Building_List_Response;
        foreach (var building in res.Buildings)
        {
          if (building.BuildingID == 8002)
          {
            buildingUID1 = building.BuildingUID;
          }
          else if (building.BuildingID == 8003)
          {
            buildingUID2 = building.BuildingUID;
          }
          else if (building.BuildingID == 8008)
          {
            buildingUID3 = building.BuildingUID;
          }
        }
        Move();
      });
      Wait();


      Reset();
      HttpNetMgr.RequestLandmarkRewardCoralTree(buildingUID1, (rst, obt) =>
      {
        checkRst = rst;
        var response = obt as Landmark_Reward_CoralTree_Response;
        Move();
      });
      Wait();

      Assert.Equal(checkRst, checkRst);

      Reset();
      HttpNetMgr.RequestLandmarkRewardGiantClam(buildingUID2, (rst, obt) =>
      {
        checkRst = rst;
        var response = obt as Landmark_Reward_CoralTree_Response;
        Move();
      });
      Wait();

      Assert.Equal(checkRst, checkRst);

      Reset();
      HttpNetMgr.RequestLandmarkRewardVolcano(buildingUID3, 1, (rst, obt) =>
      {
        checkRst = rst;
        var response = obt as Landmark_Reward_CoralTree_Response;
        Move();
      });
      Wait();

      Assert.Equal(checkRst, checkRst);

    }
  }
}



