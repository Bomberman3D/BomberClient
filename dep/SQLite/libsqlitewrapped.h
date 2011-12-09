/* Copyright (C) 2004,2005,2006  Anders Hedstrom

This program is made available under the terms of the GNU GPL.

If you would like to use this program in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for this program, please
visit http://www.alhem.net/sqlwrapped/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <string>
#include "sqlite3.h"

#ifndef _DATABASE_H_SQLITE
#define _DATABASE_H_SQLITE

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <string>
#include <list>
#ifdef WIN32
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif

#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif

class Query;
class Mutex;

class Database 
{
public:
    class Mutex
    {
    public:
        Mutex();
        ~Mutex();
        void Lock();
        void Unlock();
    private:
#ifdef _WIN32
        HANDLE m_mutex;
#else
        pthread_mutex_t m_mutex;
#endif
    };
private:
    class Lock
    {
    public:
        Lock(Mutex& mutex,bool use);
        ~Lock();
    private:
        Mutex& m_mutex;
        bool m_b_use;
    };
public:
    struct OPENDB
    {
        OPENDB() : busy(false) {}
        sqlite3 *db;
        bool busy;
    };
    typedef std::list<OPENDB *> opendb_v;

public:
    Database(const std::string& database);

    Database(Mutex& ,const std::string& database);

    virtual ~Database();

    bool Connected();

    void error(Query&,const char *format, ...);
    void error(Query&,const std::string& );

    OPENDB *grabdb();
    void freedb(OPENDB *odb);

    std::string safestr(const std::string& );
    std::string xmlsafestr(const std::string& );

    int64_t a2bigint(const std::string& );
    uint64_t a2ubigint(const std::string& );

private:
    Database(const Database& ) : m_mutex(m_mutex) {}
    Database& operator=(const Database& ) { return *this; }
    void error(const char *format, ...);

    std::string database;
    opendb_v m_opendbs;
    bool m_embedded;
    Mutex& m_mutex;
    bool m_b_use_mutex;
};

#ifdef SQLITEW_NAMESPACE
}
#endif

#endif // _DATABASE_H
#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#ifndef _QUERY_H_SQLITE
#define _QUERY_H_SQLITE

#include <string>
#include <map>
#ifdef WIN32
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif

class Query 
{
public:
    Query(Database& dbin);
    Query(Database& dbin,const std::string& sql);
    ~Query();

    bool Connected();
    Database& GetDatabase() const;
    const std::string& GetLastQuery();

    bool execute(const std::string& sql);

    sqlite3_stmt *get_result(const std::string& sql);
    void free_result();
    bool fetch_row();
    sqlite_int64 insert_id();
    long num_rows();
    int num_cols();
    std::string GetError();
    int GetErrno();

    bool is_null(int x);

    const char *get_string(const std::string& sql);
    long get_count(const std::string& sql);
    double get_num(const std::string& sql);

    const char *getstr(const std::string& x);
    const char *getstr(int x);
    const char *getstr();

    long getval(const std::string& x);
    long getval(int x);
    long getval();

    unsigned long getuval(const std::string& x);
    unsigned long getuval(int x);
    unsigned long getuval();

    int64_t getbigint(const std::string& x);
    int64_t getbigint(int x);
    int64_t getbigint();

    uint64_t getubigint(const std::string& x);
    uint64_t getubigint(int x);
    uint64_t getubigint();

    double getnum(const std::string& x);
    double getnum(int x);
    double getnum();

private:
    Query(const Query& q) : m_db(q.GetDatabase()) {}
    Query& operator=(const Query& ) { return *this; }
    void ViewRes();
    void error(const std::string& );
    Database& m_db;
    Database::OPENDB *odb;
    sqlite3_stmt *res;
    bool row;
    short rowcount;
    std::string m_tmpstr;
    std::string m_last_query;
    int cache_rc;
    bool cache_rc_valid;
    int m_row_count;

    std::map<std::string,int> m_nmap;
    int m_num_cols;
};


#ifdef SQLITEW_NAMESPACE
}
#endif

#endif // _QUERY_H
