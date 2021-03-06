
#include "Search_Compatibility.h"
#include "sql_queries.h"

bool Search_Compatibility::retrieve_internal(long &category2, long &resulting_category)
{
    bool ok;
    category2 =query.value(0).toULongLong(&ok);
    if (!ok)
    {
        _error << "Unexpected Error: Non-integer category_id's\n";
        return false;
    }
    if (query.value(1).isNull())
        resulting_category=-1;
    else
    {
        resulting_category=query.value(1).toULongLong(&ok);
        if (!ok)
        {
            _error << "Unexpected Error: Non-integer category_id's\n";
            return false;
        }
    }
    return true;
}

QString Search_Compatibility::getInflectionRules(){
    return query.value(2).toString();
}

Search_Compatibility::Search_Compatibility(rules rule, long category_id,bool first)
{
    err=false;
    QSqlQuery temp(theSarf->db);
    query=temp;
    this->rule=rule;
    item_types t1,t2,t_cat;
    if (!get_types_of_rule(rule,t1,t2))
        err=true;
    get_type_of_category(category_id,t_cat);
    if ((first?t1:t2)!=t_cat)
        err=true;
    QString stmt( "SELECT %4, resulting_category,inflections FROM compatibility_rules WHERE %3 ='%1' AND type=%2");
    stmt=stmt.arg(category_id).arg((int)rule).arg(first?"category_id1":"category_id2").arg(first?"category_id2":"category_id1");
    //qDebug()<<stmt;
    if (!execute_query(stmt,query)) //will use the local query
        err=true;
}
int Search_Compatibility::size() //total size and not what is left
{
    return query.size();
}
bool Search_Compatibility::retrieve(long &category2, long &resulting_category)
{
    if (!err && query.next())
        return retrieve_internal(category2,resulting_category);
    else
        return false;
}
bool Search_Compatibility::retrieve(long &category2)
{
    long resulting_category;
    return retrieve(category2, resulting_category);
}
int Search_Compatibility::retrieve(long category2_ids[],long resulting_categorys[],int size_of_array)
{
    if (size_of_array<=0 || !err)
        return 0;
    int i;
    for (i=0; i<size_of_array && query.next();i++)
    {
        if (!retrieve_internal(category2_ids[i],resulting_categorys[i]))
            return -1;
    }
    return i;
}
int Search_Compatibility::retrieve(long category2_ids[],int size_of_array)
{
    long resulting_categorys[size_of_array];
    return retrieve(category2_ids,resulting_categorys,size_of_array);
}


