DROP PROCEDURE IF EXISTS p_account_update_life;

DELIMITER $$

CREATE DEFINER=root@localhost PROCEDURE p_account_update_life
(

# input
    in_gameUID              bigint unsigned,
    in_lifeUpdateTime       datetime,
    in_addLife          	int,
# output
	out out_updatedLife     int,
    out out_result          int
)
deterministic
begin
    set out_result = 0;
    update t_account set lifeUpdateTime = in_lifeUpdateTime, life = @updatedLife := life + in_addLife where gameUID = in_gameUID;

    set @ret = row_count();
    set out_updatedLife = @updatedLife;
    if @ret = 0 then set out_result = 3; end if;
end $$

DELIMITER ;