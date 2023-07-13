/* 
 * File:   sqlite_lib.h
 * Author: Peggy Chang
 *
 * Created on 2011�~8��16��, �U�� 2:00
 */

#ifndef _SQLITE_LIB_H
#define	_SQLITE_LIB_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdarg.h>

#include <typedef.h>

typedef char DataBaseName[16];
typedef char TableName[16];
typedef char FieldData[32];
typedef char FieldName[16];

enum DataType
{
   _INTEGER,
   _TEXT,
   _REAL,
   _BLOB,
   _NULL,
   _NUMERIC,
};

enum Constraint
{
    _NONE = 0x00,
    _NOT_NULL = 0x01,
    _PRIMARY_KEY = 0x02,
    _UNIQUE = 0x04,
    _AUTOINCREMENT = 0x08,
};

typedef struct
{
    char FieldName[64];
    enum DataType emDataType;
    enum Constraint emConstraint;
}FieldDef;

void TSQLite_Initial(void);
void TSQLite_Close(void);
BOOL TSQLite_Connect(DataBaseName ansDataBaseName);
void TSQLite_DisConnect(void);
BOOL TSQLite_Exec(char *cmd, int (*callback)(void *NotUsed, int argc, char **argv, char **azColName));
BOOL TSQLite_CreateTable(TableName ansTableName, int FieldCount, FieldDef *stFieldDefs);
BOOL TSQLite_DropTable(TableName ansTableName);
BOOL TSQLite_DeleteAllRecrod(TableName ansTableName);
BOOL TSQLite_Append(TableName ansTableName, int DataCount, FieldData *FieldData);
BOOL TSQLite_UpdateByCondition(TableName ansTableName, BYTE *ansCondition, int DataCount, FieldName *FieldName, FieldData *FieldData);
BOOL TSQLite_Update(TableName ansTableName,FieldName FieldName,FieldData FieldData);
BOOL TSQLite_Query(TableName ansTableName);
BOOL TSQLite_QueryByCondition(TableName ansTableName, BYTE *ansCondition);
BOOL TSQLite_OrderBy(TableName ansTableName, FieldName ansOrderBy_FieldName);
BOOL TSQLite_Delete(TableName ansTableName,FieldName ansFieldName,BYTE *ansCondition);
BOOL TSQLite_DeleteByCondition(TableName ansTableName, BYTE *ansCondition);
BOOL TSQLite_Transaction(BYTE bMode);

void (*OnError)(BYTE *ErrMsg);
void (*OnRecords)(int argc, char **argv, char **azColName);

#ifdef	__cplusplus
}
#endif

#endif	/* _SQLITE_LIB_H */

