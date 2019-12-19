DROP PROCEDURE IF EXISTS p_account_find;

DELIMITER $$

CREATE DEFINER=root@localhost PROCEDURE p_account_find
(

# input
    in_gameUID                      bigint unsigned,
# output
    OUT out_profileID               smallint,
    OUT out_worldName               varchar(50),
    OUT out_level                   smallint,
    OUT out_rewardedLevel           smallint,
    OUT out_exp                     int,
    OUT out_life                    int,
    OUT out_ao                      int,
    OUT out_coral                   int,
    OUT out_seaweed                 int,

    OUT out_fishAir                 int,
    OUT out_buildingAir             int,
    OUT out_buildingTemperature     int,
    OUT out_buildingClean           int,

    OUT out_areaAir                 int,
    OUT out_areaClean               int,
    OUT out_areaTemperature         int,


    OUT out_chargeCash              int,
    OUT out_eventCash               int,
    OUT out_storageLevel            smallint,
    OUT out_shareFishID             smallint,
    OUT out_heart                   int,
    OUT out_socialPoint             int,

    OUT out_openArea                tinyint unsigned,

    OUT out_lifeUpdateTime          datetime,
    OUT out_lastUpdateTime          datetime,
    OUT out_result                  int
)
    DETERMINISTIC
begin
    declare continue handler for not found set @ret = 0;
    
    set out_result = 0;
    
    select profileID, worldName, `level`, rewardedLevel, `exp`, life, ao, coral, seaweed, fishAir, buildingAir, buildingTemperature, buildingClean, areaAir, areaClean, areaTemperature, chargeCash, eventCash, storageLevel, shareFishID, heart, socialPoint, openArea, lifeUpdateTime, lastUpdateTime
    into out_profileID, out_worldName, out_level, out_rewardedLevel, out_exp, out_life, out_ao, out_coral, out_seaweed, out_fishAir, out_buildingAir, out_buildingTemperature, out_buildingClean, out_areaAir, out_areaClean, out_areaTemperature, out_chargeCash, out_eventCash, out_storageLevel, out_shareFishID, out_heart, out_socialPoint, out_openArea, out_lifeUpdateTime, out_lastUpdateTime

    from t_account
    where gameUID = in_gameUID;
    
    set @ret = row_count();

    if @ret = 0 then set out_result = 1; end if;
end $$

DELIMITER ;
