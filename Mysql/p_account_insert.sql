DROP PROCEDURE IF EXISTS p_account_insert;

DELIMITER $$

CREATE DEFINER=root@localhost PROCEDURE p_account_insert
(
# input
        in_gameUID                  bigint unsigned,
        in_worldName                varchar(50),
        in_currTime                 datetime,
        in_level                    smallint,
        in_exp                      int,
        in_life                     int,
        in_ao                       int,
        in_coral                    int,
        in_seaweed                  int,

        in_areaAir              int,
        in_areaClean            int,
        in_areaTemperature      int,

        in_eventCash                int,
        in_openArea                 tinyint unsigned,
        in_lifeUpdateTime           datetime,

        in_buildingQuery            varchar(4096),
        in_itemQuery                varchar(4096),
        in_expeditionQuery          varchar(4096),

        in_sequenceType             tinyint unsigned,
        in_lastBuildingUID          int,
        in_fishRecipeID             smallint,
# output
    out out_result  int
)
deterministic
begin
    DECLARE EXIT HANDLER FOR SQLEXCEPTION set out_result = 2;
    set out_result = 0;

    insert into t_sequence (gameUID, `type`, seq) values(in_gameUID, in_sequenceType, in_lastBuildingUID);

    insert into t_account(gameUID, worldName, `level`, `exp`, life, ao, coral, seaweed, areaAir, areaClean, areaTemperature, eventCash, registTime, openArea, lifeUpdateTime)
        values (in_gameUID, in_worldName, in_level, in_exp, in_life, in_ao, in_coral, in_seaweed, in_areaAir, in_areaClean, in_areaTemperature, in_eventCash,  in_currTime, in_openArea, in_lifeUpdateTime);

    insert into t_fish_recipe (gameUID, recipeID, `count`) values(in_gameUID, in_fishRecipeID, 1);

    INSERT INTO t_quest(gameUID, mainID) VALUES(in_gameUID, 0);

    INSERT INTO t_landmark(gameUID) VALUES(in_gameUID);
    set @sql = in_buildingQuery;
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    set @sql = in_itemQuery;
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    set @sql = in_expeditionQuery;
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

end $$

DELIMITER ;
