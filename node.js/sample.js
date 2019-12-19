app.get('FinishBattle', function FinishChampionship(req, res, next) {
    var context = this;
    var kingdom = context.session.kingdom;
    var manager = championship_manager;
    var data_key = get_kingdom_data_key(kingdom, 'championship');
    var season = manager.get_active_season();
    var cool_time_hour = meta.GameConfig.championship_field_cool_time_hour;
    var data = {
        key: null, //uid(32),
        units: null,//null,
        season_id: null,//null,
        slot: null,//req.slot,
        startTime: null,//(new Date()).getTime(),
        battlePower: 0, //uint32
    };
    var heroes = null;
    var relics = null;
    var battle = null;
    
    var field = {
        units: null,
        relics: null,
        fieldRemainSeconds: null,
        battlePower: 0,
    };
    var saveFieldTime = moment();
    var match = null;
    var rule = {
        placement: meta.GameConfig.championship_game_count.placement,
        ascending: meta.GameConfig.championship_game_count.ascending,
        descending: meta.GameConfig.championship_game_count.descending,
    };
    var before_score = 0;
    var before_tier = '';
    var tier_info = null;
    var history = null;

    function load() {
        if (!season) {
            throw new BadRequest('season is not active');
        }

        return Promise.props({
            data: checkKeyData(data_key, req.championshipKey),
            heroes: loadBattleForceAsync(context, kingdom, req.battleForce),
            relics: loadAndCheckRelics(context, kingdom, req.relicIDs),
            appguard: kingdom._appguard_send_data ? appguard_auth.checkDataAsync(context, req.appguardResponse) : false,
        }).then(function(result) {

            if (!result.data)
                throw new BadRequest('data not found');

            data = result.data;
            heroes = result.heroes;
            relics = result.relics;

            var season_id = data.season_id;

            return Promise.props({
                battle: manager.getUserMatchAsync(kingdom.id, season_id),
                score: manager.getUserScoreAsync(kingdom.id, season_id),
                field: manager.getUserFieldAsync(kingdom.id, season_id),
            })
            .then(function(result) {
                battle = result.battle;
                battle.score = result.score || 0;
                field = result.field;

                before_score = battle.score;
                before_tier = battle.tier;
            });
        });
    }

    function validate() {
        if (!battle)
            throw new BadRequest('championship not found');

        var matches = battle.matches;
        var found = matches.some(function(match_data) {
            if (match_data.slot == data.slot) {
                match = match_data;
                return true;
            }
            return false;
        });

        if (!found)
            throw new BadRequest('invalid slot');

        tier_info = meta.ChampionshipTierInfo.get_by_id(battle.tier);

        if(battle.trialKind != 'placement') {
            if (!tier_info)
                throw new BadRequest('tier_info not found');
        }
       
        if (req.battlePower != data.battlePower) {
            throw new BadRequest('invalid battlePower');
        }

        if (req.saveField) {
            if (field && field.fieldRemainSeconds) {
                if (field.fieldRemainSeconds > 0)
                    throw new BadRequest('invalid save request');
            }
        }
    }

    function battle_log() {
        return battleLog(context, req, int(data.startTime), heroes, match.units);
    }

    function work() {

        function normal() {

            if (req.result === 'win') {
                battle.winCount += 1;
                battle.serial += 1;
            } else {
                battle.loseCount += 1;
                battle.serial = 0;
            }

            var is_win = req.result === 'win';

            battle.score = gameutil.calculation_elo_point(battle.score, match.score, is_win);
            if (battle.score < 0) {
                battle.score = 0;
            }

            LOG.debug('elo %d', battle.score);

            var after_score = battle.score;

            if (before_score < after_score) {
                if (tier_info.score_max < after_score) {
                    var next_tier = tier_info.next();
                    if (next_tier) {
                        battle.trialKind = 'ascending';
                    }
                }
            } else {
                if (tier_info.score_min > after_score) {
                    var previous_tier = tier_info.previous(battle.tier);
                    if (previous_tier) {
                        battle.trialKind = 'descending';
                    }
                }
            }
        }

        function placement() {

            battle.trialRecords.push(req.result);

            if (rule.placement <= battle.trialRecords.length) {
                var placement_result = meta.ChampionshipPlacementForce.get_placement_result(battle.trialRecords);

                battle.trialRecords = [];
                battle.trialKind = 'normal';
                battle.tier = placement_result.tier;
                battle.score = placement_result.score;
            }
        }

        function change(type) {

            battle.trialRecords.push(req.result);
            var win_count = 0;
            var lose_count = 0;
            var add = 0;
            var remove = 0;
            var all = 0;

            var records = battle.trialRecords;

            if(type.ascending) {
                if (rule.ascending <= battle.trialRecords.length) {
                    check_match();

                    var next = tier_info.next();
                    if (next) {
                        add = tier_info.ascend_win_score * win_count;
                        remove = tier_info.ascend_lose_score * lose_count;
                        all = add + remove;

                        if (all) {
                            battle.tier = next.id;
                        }
                        battle.score += all;
                    }

                    reset();
                }
            }

            if(type.descending) {
                if (rule.descending <= battle.trialRecords.length) {
                    check_match();

                    var previous = tier_info.previous();
                    if (previous) {
                        add = (tier_info.descend_win_score * win_count);
                        remove = (tier_info.descend_lose_score * lose_count);
                        all = add + remove;

                        if (all < 0) {
                            battle.tier = previous.id;
                        }
                        battle.score += all;
                    }

                    reset();
                }
            }

            function reset() {
                battle.trialRecords = [];
                battle.trialKind = 'normal';
            }

            function check_match() {
                records.forEach(function(record) {
                    if (record === 'win') {
                        win_count += 1;
                    }
                    else {
                        lose_count += 1;
                    }
                });
            }
        }

        if (req.saveField) {
            field.fieldRemainSeconds = moment(saveFieldTime).add(cool_time_hour, 'hours').diff(moment(), 'seconds');
            field.units = me.defenseForceUnits2WaveForceUnits(context, req.battleForce, heroes, {skill:true, item:true});
            field.relics = me.make_battle_relic(relics);
            field.battlePower = req.battlePower;
        }

        switch(battle.trialKind) {
            case 'placement':
                placement();
                break;

            case 'ascending':
                change({ascending:true});
                break;

            case 'descending':
                change({descending:true});
                break;

            default:
                normal();
                break;
        }

        battle.matches = battle.matches.filter(function(data) {
            return match.slot != data.slot;
        });

        var forces = {
            attacker: field.units,
            defender: match.units,
        };

        history = db.global.models.championship_history.make(kingdom.id, 'championship', req.result, forces, req.replay, before_score, battle.score);
    }

    function save_write() {

        function get_status() {
            var fieldRemainSeconds = -1;
            if (field && field.fieldRemainSeconds) {
                fieldRemainSeconds = field.fieldRemainSeconds;
            }

            return {
                tier: battle.tier,
                trialKind: battle.trialKind,
                trialRecords: battle.trialRecords,
                winCount: battle.winCount,
                loseCount: battle.loseCount,
                serialCount: battle.serial,
                units: field.units,
                relics: field.relics,
                fieldRemainSeconds: fieldRemainSeconds,
            };
        }

        var season_id = data.season_id;
        var world_id = context.server.world_id;
        var jobs = [
            manager.setUserMatchAsync(kingdom.id, season_id, battle),
            manager.setUserAsync(kingdom.id, getUserObject(kingdom))
        ];

        if (req.saveField) {
            var saveObject = {
                field: JSON.stringify({
                    u: field.units,
                    r: field.relics,
                }),
                fieldTime: saveFieldTime.unix(),
                battlePower: field.battlePower,
            };
            jobs.push(manager.setUserFieldAsync(kingdom.id, season_id, saveObject));
        }

        if (battle.trialKind === 'normal') {
            jobs = jobs.concat([
                manager.setUserRankAsync(kingdom.id, season_id, battle.score),
                manager.setTierRankAsync(kingdom.id, battle.tier, season_id, battle.score),
                manager.setWorldRankAsync(kingdom.id, world_id, season_id, battle.score),
            ]);
        }

        if (before_tier !== battle.tier) {
            jobs.push(manager.deleteTierRankAsync(kingdom.id, before_tier, season_id));
        }

        return Promise.all(jobs).then(function() {
            context.write('kingdom', kingdom);
            context.write('battleStatus', get_status());
            context.write('forces', battle.matches);
            context.write('historyID', history.id);

            context.save(history);
        });
    }

    function log() {
        context.activity({
            strings: [req.result, before_tier, battle.tier],
            integers: [before_score, battle.score],
        });
    }

    return load()
        .then(validate)
        .then(battle_log)
        .then(work)
        .then(save_write)
        .then(log)
        .then(next, next);
});