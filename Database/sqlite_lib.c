#include <string.h>
#include <stdio.h>

#include <sqlite3.h>
//#include "../Includes/Sqlite_lib.h"

static int rtn;
static char str[512];
static sqlite3 *db;

void TSQLite_Initial(void)
{
    db = NULL;
    OnError = NULL;
    OnRecords = NULL;
}

void TSQLite_Close(void)
{
    if (db != NULL) sqlite3_close(db);
    OnError = NULL;
    OnRecords = NULL;
    db = NULL;
}

BOOL TSQLite_Connect(DataBaseName ansDataBaseName)
{
    if (db != NULL) return FALSE;

    rtn = sqlite3_open(ansDataBaseName, &db);
    if (rtn)
    {
        sqlite3_close(db);
        db = NULL;
        return FALSE;
    }

    return TRUE;
}

void TSQLite_DisConnect(void)
{
   if (db != NULL) sqlite3_close(db);
   db = NULL;
}

static int callback_no(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (OnRecords != NULL)
      OnRecords(argc, argv, azColName);
    
    //return 0;
};

static int callback_query(void *NotUsed, int argc, char **argv, char **azColName){
  //int i;
  //char str[100];

  if (OnRecords != NULL)
      OnRecords(argc, argv, azColName);
//  sprintf(str, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

  return 0;
}

BOOL TSQLite_Exec(char *cmd, int (*callback)(void *NotUsed, int argc, char **argv, char **azColName))
{
   char *zErrMsg = 0;
   BYTE buff[128];

   rtn = sqlite3_exec(db, cmd, callback, 0, &zErrMsg);
   //DebugAddINT("rtn", rtn);
   if( rtn!=SQLITE_OK ){
       memset(buff, 0, sizeof(buff));
       sprintf((char *)buff, "SQL error: %s", zErrMsg);
       //DebugAddSTR("MSG",zErrMsg,32);
        if (OnError != NULL) OnError(buff);
        sqlite3_free(zErrMsg);
        return FALSE;
   }

   return TRUE;
};

BOOL TSQLite_CreateTable(TableName ansTableName, int FieldCount, FieldDef *stFieldDefs)
{
    int i;
    
    memset(str, 0, sizeof(str));
    sprintf(str, "create table IF NOT EXISTS %s (", ansTableName);

    for (i=0; i<FieldCount; i++)
    {
        memcpy(&str[strlen(str)], stFieldDefs[i].FieldName, strlen(stFieldDefs[i].FieldName));
        switch(stFieldDefs[i].emDataType)
        {
            case _INTEGER:
                sprintf(&str[strlen(str)], " INTEGER");
                break;
            case _TEXT:
                sprintf(&str[strlen(str)], " TEXT");
                break;
            case _REAL:
                sprintf(&str[strlen(str)], " REAL");
                break;
            case _BLOB:
                sprintf(&str[strlen(str)], " BLOB");
                break;
            case _NULL:
                sprintf(&str[strlen(str)], " NULL");
                break;
        }

        if ((stFieldDefs[i].emConstraint & _NOT_NULL) == _NOT_NULL)
            sprintf(&str[strlen(str)], " NOT NULL");

        if ((stFieldDefs[i].emConstraint & _PRIMARY_KEY)== _PRIMARY_KEY)
        {
            sprintf(&str[strlen(str)], " PRIMARY KEY");

            if ((stFieldDefs[i].emConstraint & _AUTOINCREMENT) == _AUTOINCREMENT)
                sprintf(&str[strlen(str)], " AUTOINCREMENT");
        }

        if ((stFieldDefs[i].emConstraint & _UNIQUE) == _UNIQUE)
            sprintf(&str[strlen(str)], " UNIQUE");

        if (i != FieldCount-1)
            str[strlen(str)] = ',';
        else
            str[strlen(str)] = ')';
    }

    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_DropTable(TableName ansTableName)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "drop table IF EXISTS %s", ansTableName);

     if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_DeleteAllRecrod(TableName ansTableName)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "delete from %s", ansTableName);

     if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_Append(TableName ansTableName, int DataCount, FieldData *FieldData)
{
    int i;
    
    memset(str, 0, sizeof(str));
    sprintf(str, "insert into %s values (", ansTableName);

    for (i=0; i<DataCount; i++)
    {
        sprintf(&str[strlen(str)], "\"%s\"", FieldData[i]);
        if (i != DataCount-1)
            sprintf(&str[strlen(str)], " ,");
        else
            str[strlen(str)] = ')';
    }

    //DebugAddHEX("insert str", str, strlen(str));
    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_UpdateByCondition(TableName ansTableName, BYTE *ansCondition, int DataCount, FieldName *FieldName, FieldData *FieldData)
{
    int i;
    memset(str, 0, sizeof(str));
    sprintf(str, "update  %s set ", ansTableName);

    for (i=0; i<DataCount; i++)
    {
        sprintf(&str[strlen(str)], "%s =\"%s\"", FieldName[i], FieldData[i]);

        if (i == DataCount-1) break;

        sprintf(&str[strlen(str)], ", ");
    }

    sprintf(&str[strlen(str)], "where %s", ansCondition);

    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_Update(TableName ansTableName,FieldName FieldName,FieldData FieldData)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "update  %s set %s=%s", ansTableName, FieldName, FieldData);
    
    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_Query(TableName ansTableName)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "select * from %s", ansTableName);

     if (!TSQLite_Exec(str, callback_query)) return FALSE;

    return TRUE;
}

BOOL TSQLite_QueryByCondition(TableName ansTableName, BYTE *ansCondition)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "select * from %s where %s", ansTableName, ansCondition);

     if (!TSQLite_Exec(str, callback_query)) return FALSE;

    return TRUE;
}

BOOL TSQLite_OrderBy(TableName ansTableName, FieldName ansOrderBy_FieldName)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "select * from %s order by %s", ansTableName, ansOrderBy_FieldName);

    if (!TSQLite_Exec(str, callback_query)) return FALSE;

    return TRUE;
}

BOOL TSQLite_Delete(TableName ansTableName,FieldName ansFieldName,BYTE *ansCondition)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "delete from %s where %s=%s", ansTableName, ansFieldName, ansCondition);

    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_DeleteByCondition(TableName ansTableName, BYTE *ansCondition)
{
    memset(str, 0, sizeof(str));
    
 //   DebugAddHEX("ansCondition", ansCondition, strlen(ansCondition));
    sprintf(str, "delete from %s where %s", ansTableName, ansCondition);

  //  DebugAddHEX("str", str, strlen(str));
    
    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

BOOL TSQLite_Transaction(BYTE bMode)
{
    memset(str, 0, sizeof(str));
    switch(bMode)
    {
        case 0x00:
            sprintf(str, "begin");
            break;
        case 0x01:
            sprintf(str, "commit");
            break;
        case 0x02:
            sprintf(str, "rollback");
            break;
    }

    if (!TSQLite_Exec(str, callback_no)) return FALSE;

    return TRUE;
}

