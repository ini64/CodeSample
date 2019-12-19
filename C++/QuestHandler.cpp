#include "stdafx.h"

AcInt32 AcCmdCLRequestDailyQuestListHandler::Execute(AcSession& pCli, AcInputMsg& pMsg)
{
    if (AcMasterSession::INSTANCE()->IsValid() == FALSE)
    {
        LOGF("Master Session has been closed!\n");
        return 0;
    }

    USER_UID userUid = m_Cmd.GetUserUID();

    LOG("REQUEST DAILY QUEST LIST: USER_UID($1)\n", userUid);

    AcSendMsg msg;
    Writer(msg, userUid);

    if (AcMasterSession::INSTANCE()->Send(LM_REQUEST_DAILY_QUEST_LIST, msg) == FALSE)
    {
        LOGF("Master Session has been closed!\n");
        return 0;
    }

    return 0;
}

AcInt32 AcCmdLMRequestDailyQuestListOKHandler::Execute(AcSession& pCli, AcInputMsg& pMsg)
{
    USER_UID userUid = m_Cmd.GetUserUID();

    AcString3<AcBUFSIZ> strQuest;
    strQuest.Format("QUEST_LIST");

    const AcUser* user = AcUserDepot::INSTANCE()->FindUser(userUid);
    if (user != NULL && user->HasSession())
    {
        AcCmdCLRequestDailyQuestListOK cmd;
        cmd.ResetList();
        cmd.SetUserUID(userUid);

        strQuest.Append("(count1:$1)", m_Cmd.GetCountQuest());

        for (RwUSize i = 0; i < m_Cmd.GetCountQuest(); ++i)
        {
            const sAchievedItem& item = m_Cmd.GetDailyQuestInfo(i);
            cmd.PushDailyQuest(item);

            strQuest.Append(",($1,$2,$3)", item.achv_tid, item.countOrValue, item.rewardType);
        }

        strQuest.Append(",(count2:$1)", m_Cmd.GetCountQuestGroup());

        for (RwUSize i = 0; i < m_Cmd.GetCountQuestGroup(); ++i)
        {
            const sAchievedItem& item = m_Cmd.GetDailyQuestGroupInfo(i);
            sDailyQuestInfo quest = {0};
            quest.questTid = item.achv_tid;
            quest.questValue = item.countOrValue;
            quest.rewardType = item.rewardType;
            quest.updateType = item.updateType;

            cmd.PushDailyQuestGroup(quest);

            strQuest.Append(",($1,$2,$3)", item.achv_tid, item.countOrValue, item.rewardType);
        }

        user->Send(cmd);
    }

    LOG("REQUEST DAILY QUEST LIST - OK: USER_UID($1),$2\n", userUid, strQuest);

    return 0;
}

AcInt32 AcCmdCLRequestQuestListHandler::Execute(AcSession& pCli, AcInputMsg& pMsg)
{
    if (AcMasterSession::INSTANCE()->IsValid() == FALSE)
    {
        LOGF("Master Session has been closed!\n");
        return 0;
    }

    USER_UID userUid = m_Cmd.GetUserUID();

    LOG("REQUEST QUEST LIST: USER_UID($1)\n", userUid);

    AcSendMsg msg;
    Writer(msg, userUid);

    if (AcMasterSession::INSTANCE()->Send(LM_REQUEST_QUEST_LIST, msg) == FALSE)
    {
        LOGF("Master Session has been closed!\n");
        return 0;
    }

    return 0;
}

AcInt32 AcCmdLMRequestQuestListOKHandler::Execute(AcSession& pCli, AcInputMsg& pMsg)
{
    USER_UID userUid = m_Cmd.GetUserUID();
    AcInt16 keyQuestId = m_Cmd.GetKeyQuestId();
    QUEST_TID keyQuestTid = m_Cmd.GetKeyQuestTid();

    AcString3<AcBUFSIZ> strQuest;
    strQuest.Format("QUEST_LIST");

    const AcUser* user = AcUserDepot::INSTANCE()->FindUser(userUid);
    if (user != NULL && user->HasSession())
    {
        AcCmdCLRequestQuestListOK cmd;
        cmd.ResetList();
        cmd.SetUserUID(userUid);
        cmd.SetKeyQuestId(keyQuestId);
        cmd.SetKeyQuestTid(keyQuestTid);

        strQuest.Append("(count:$1)", m_Cmd.GetCountQuest());

        for (RwUSize i = 0; i < m_Cmd.GetCountQuest(); ++i)
        {
            const sAchievedItem& item = m_Cmd.GetQuestInfo(i);
            cmd.PushQuest(item);
        }

        user->Send(cmd);
    }

    LOG("REQUEST QUEST LIST - OK: USER_UID($1),$2\n", userUid, strQuest);

    return 0;
}
