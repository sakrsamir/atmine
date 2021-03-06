#include <QFile>
#include <QStringList>
#include <QDir>
#include <QString>
#include <QRegExp>
#include "Retrieve_Template.h"
#include "Search_by_category.h"
#include "functions.h"
#include "sql_queries.h"
#include "text_handling.h"
#include "diacritics.h"
#include "Search_Compatibility.h"
//#include "test.h"

bool pos = false;
long abstract_possessive;

int insert_rules_for_Nprop_Al() {
    // copies all rules of Nprop to Aprop_Al and adds to them the support for Al
    int source_id = insert_source("Jad and Hamza modifications", "as needed", "Hamza Harkous and Jad Makhlouta");
    QStringList added_prefixes;
    added_prefixes << "NPref-Al"
                   << "NPref-BiAl"
                   << "NPref-Lil"
                   << "NPref-LiAl";
    QStringList added_suffixes;
    added_suffixes << "NSuff-AF";
    long prefix_cat_id, suffix_cat_id;
    long stem_cat_id_Nprop = getID("category", "Nprop");
    long stem_cat_id_Nprop_Al = insert_category("Nprop_Al", STEM, source_id, false);
    Search_Compatibility search_suffixes(BC, stem_cat_id_Nprop, true);

    while (search_suffixes.retrieve(suffix_cat_id)) {
        Search_Compatibility search_prefixes(AB, stem_cat_id_Nprop, false);

        while (search_prefixes.retrieve(prefix_cat_id)) {
            insert_compatibility_rules(AB, prefix_cat_id, stem_cat_id_Nprop_Al, source_id);
            insert_compatibility_rules(AC, prefix_cat_id, suffix_cat_id, source_id);
            insert_compatibility_rules(BC, stem_cat_id_Nprop_Al, suffix_cat_id, source_id);
        }

        QString prefix_cat;
        foreach(prefix_cat, added_prefixes) {
            prefix_cat_id = getID("category", prefix_cat);
            insert_compatibility_rules(AB, prefix_cat_id, stem_cat_id_Nprop_Al, source_id);
            insert_compatibility_rules(AC, prefix_cat_id, suffix_cat_id, source_id);
            insert_compatibility_rules(BC, stem_cat_id_Nprop_Al, suffix_cat_id, source_id);
        }
    }

    Search_Compatibility search_prefixes(AB, stem_cat_id_Nprop, false);

    while (search_prefixes.retrieve(prefix_cat_id)) {
        QString suffix_cat;
        foreach(suffix_cat, added_suffixes) {
            suffix_cat_id = getID("category", suffix_cat);
            insert_compatibility_rules(AB, prefix_cat_id, stem_cat_id_Nprop_Al, source_id);
            insert_compatibility_rules(AC, prefix_cat_id, suffix_cat_id, source_id);
            insert_compatibility_rules(BC, stem_cat_id_Nprop_Al, suffix_cat_id, source_id);
        }
    }

    return 0;
}

// utility function for insert scripts used in insert_propernames() and insert_placenames()
int insert_NProp(QString word, QList<long> *abstract_categories, int source_id, QString description = "Proper noun") {
    bool hasAL = removeAL(word);
    // insert word as is
    int stem_id = insert_item(STEM,  removeDiacritics(word), word, (hasAL ? "Nprop_Al" : "Nprop_Al"),
                              source_id, abstract_categories, description, "", "", "");

    if (stem_id < 0) {
        _error << "while adding stem: " << word << "'\n";
        return -1;
    }

    // insert possessive form of the word
    if (!pos) {
        abstract_possessive = insert_category("POSSESSIVE", STEM, dbitvec(max_sources), true);
        pos = true;
    }

    abstract_categories->append(abstract_possessive);
    QString possessive = get_Possessive_form(word);
    stem_id = insert_item(STEM, removeDiacritics(possessive), possessive,
                          (possessive.startsWith(lam) ? "Nall_L" : "Nall"),
                          source_id, abstract_categories,
                          QString("possessive form of ").append(description),
                          "", "", "");

    if (stem_id < 0) {
        _error << "while adding Possessive: " << word << "'\n";
        return -1;
    }

    return 0;
}

void findCategoryIds(item_types type, QString expression, QList<long> &category_ids) {
    expression.replace(QRegExp("((?:AND|OR|NOT)\\s*\\(*)\\s*\""), "\\1 name LIKE \"");
    int i1 = expression.indexOf("\"");
    int i2 = expression.indexOf("name");

    if (i1 < i2 || (i2 < 0 && i1 >= 0)) {
        expression.prepend("name LIKE ");
    }

    expression.replace("\\", "\\\\");
    expression.replace("%", "\\%");
    expression.replace("_", "\\_");
    expression.replace("*", "%");
    theSarf->out << "expression: {" << expression << "} was expanded to: <";
    QString where_condition = QString("type=%1 AND abstract=0 AND %2").arg((int)type).arg(expression);
    Retrieve_Template t("category", "id", "name", where_condition);

    while (t.retrieve()) {
        long id = t.get(0).toLongLong();
        QString cat = t.get(1).toString();
        category_ids.append(id);
        theSarf->out << cat << ",";
    }

    theSarf->out << ">\n";
    assert(category_ids.size() > 0);
}

int insertRuleAccordingToExpression(item_types type, QString cat1, QString cat2,
                                    QString resCat, QString inflections, int source_id) {
    rules rule = (type == PREFIX ? AA : CC);
    QList<long> catIds1, catIds2;
    bool exp1 = cat1.startsWith("{") && cat1.endsWith("}"),
         exp2 = cat2.startsWith("{") && cat2.endsWith("}"),
         exp3 = resCat.startsWith("{") && resCat.endsWith("}");

    if (exp1) {
        findCategoryIds(type, cat1.mid(1, cat1.size() - 2), catIds1);
    } else {
        long cat_id = insert_category(cat1, type, source_id, false);
        catIds1.append(cat_id);
    }

    if (exp2) {
        findCategoryIds(type, cat2.mid(1, cat2.size() - 2), catIds2);
    } else {
        long cat_id = insert_category(cat2, type, source_id, false);
        catIds2.append(cat_id);
    }

    for (int i = 0; i < catIds1.size(); i++) {
        for (int j = 0; j < catIds2.size(); j++) {
            long cat_id1 = catIds1[i];
            long cat_id2 = catIds2[j];

            if (cat_id1 == cat_id2) {
                continue;
            }

            long cat_idr;

            if (exp3) {
                if (resCat == "{$1}") {
                    cat_idr = cat_id1;
                } else if (resCat == "{$2}") {
                    cat_idr = cat_id2;
                } else if (resCat.startsWith("{\"") && resCat.endsWith("\"}")) {
                    QStringList l;
                    QString resCatTemp = resCat;
                    resCatTemp = resCatTemp.mid(2, resCatTemp.size() - 4);

                    for (int i = 0; i < 2; i++) {
                        long cat_id = (i == 0 ? cat_id1 : cat_id2);
                        QString catFound = getColumn("category", "name", cat_id);
                        resCatTemp.replace(QString("($%1)").arg(i + 1), catFound);
                        QRegExp r(QString("\\(\\$%1\\[(-\\d)+\\]\\)").arg(i + 1));

                        do {
                            int i = r.indexIn(resCatTemp);

                            if (i < 0) {
                                break;
                            }

                            int size = r.matchedLength();
                            int deductions = r.cap(1).toInt();
                            resCatTemp.replace(i, size, catFound.mid(0, catFound.size() + deductions));
                        } while (true);

                        bool exp = (i == 0 ? exp1 : exp2);
                        QString cat = (i == 0 ? cat1 : cat2);

                        if (exp) {
                            cat = cat.mid(2, cat.size() - 4);
                            int f = cat.indexOf("\"");

                            if (f >= 0) {
                                cat = cat.mid(0, f);
                            }

                            QRegExp reg(cat.replace("*", "(.*)"));
                            cat.append("$");
                            reg.exactMatch(catFound);
                            QStringList r = reg.capturedTexts();
                            r.removeFirst();
                            l.append(r);
                        }
                    }

                    QRegExp at("\\(@([\\d]+)\\).*");
                    int i = -1;
                    int pos = -1;

                    do {
                        pos = resCatTemp.indexOf("@", pos + 1);

                        if (pos == -1) {
                            break;
                        }

                        QString p = resCatTemp.mid(pos - 1);

                        if (at.exactMatch(p)) {
                            QString t = at.cap(1);
                            int j = t.toInt();
                            int position = pos - 1;
                            int size = t.size();

                            if (j > 0 && j <= l.size()) {
                                i = j - 1;
                            } else {
                                i++;
                                _warning << "More replacement positions (@) " "than search positions (*) in '"
                                         << resCatTemp << "'\n";
                            }

                            resCatTemp.replace(position, size + 3, l[i]);
                        } else {
                            i++;
                            resCatTemp.replace(pos, 1, l[i]);
                        }
                    } while (true);

                    cat_idr =
                        insert_category(resCatTemp, type, source_id, false);
                } else {
                    theSarf->out << "Unknown Result Category Expression \"" << resCat << "\"\n";
                    return -1;
                }
            } else {
                cat_idr = insert_category(resCat, type, source_id, false);
            }

            if (insert_compatibility_rules(rule, cat_id1, cat_id2, cat_idr,
                                           inflections, source_id) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

// main functions
int insert_buckwalter() {
    int source_id = insert_source("Buckwalter Dictionaries",
                                  "modifying aramorph.pl + insert_buckwalter() c++ code fragment",
                                  "Jad Makhlouta");
    // items
    const QString folder = "../../src/buckwalter scripts/";
    QString tag;
    const int num_files_items = 3;
    const QString item_files[num_files_items] = {
        folder + "list_of_prefixes" + tag + ".txt",
        folder + "list_of_suffixes" + tag + ".txt",
        folder + "list_of_stems.txt"
    };
    const item_types types[num_files_items] = {PREFIX, SUFFIX, STEM};

    for (int j = 0; j < num_files_items; j++) {
        int num_entries = 6;
        QFile input(item_files[j]);

        if (!input.open(QIODevice::ReadWrite)) {
            theSarf->out << "File not found\n";
            return 1;
        }

        QTextStream file(&input);
        file.setCodec("utf-8");
        int line_num = 0;

        while (!file.atEnd()) {
            line_num++;
            QString line = file.readLine(0);

            if (line.isNull()) {
                line_num--; // finished
                break;
            }

            if (line.isEmpty()) { // ignore empty lines if they exist
                continue;
            }

            QStringList entries = line.split("\t", QString::KeepEmptyParts);

            if (entries.size() < num_entries) {
                theSarf->out << "Error at line " << line_num << ": '" << line
                             << "', Insufficient tab-delimited entries\n";
                return -1;
            }

            QString item = entries[0];
            QString raw_data = entries[1];
            QString category = entries[2];
            QString description = entries[3];
            QString POS = entries[4];
            QString lemmaID = entries[5];

            if (description.contains("\"")) {
                description.replace("\"", "'");  // instead we will face some problems in insert...
            }

            // 3 letters are not properly transfered using perl script because
            // it is not based on unicode, here we solve them manually
            if (item.contains("{") || item.contains("`") || item.contains("V")) {
                item = item.replace("{", QString(alef_wasla));
                item = item.replace("`", QString(aleft_superscript));
                item = item.replace("V", QString(veh));
            }

            if (raw_data.contains("{") || raw_data.contains("`") || raw_data.contains("V")) {
                raw_data = raw_data.replace("{", QString(alef_wasla));
                raw_data = raw_data.replace("`", QString(aleft_superscript));
                raw_data = raw_data.replace("V", QString(veh));
            }

            QString abstract_category;
            long abstract_id = -1;

            if (types[j] == STEM) {
                QStringList abstract_list = POS.split("+").at(0).split("/");

                if (abstract_list.size() >= 2) {
                    abstract_category = abstract_list.at(1);
                } else {
                    abstract_category = "";
                    theSarf->out << "Unexpected Error: split on " << POS
                                 << " " << abstract_list.size() << " "
                                 << abstract_list.at(0);
                    return 1;
                }

                if ((abstract_id = getID("category", abstract_category,
                                         QString("type=%1 AND abstract=1").arg((int)STEM))) == -1) {
                    abstract_id = insert_category(abstract_category, STEM, source_id, true);
                }
            } else {
                if (!lemmaID.isEmpty() && lemmaID != "1") {
                    abstract_category = lemmaID;
                    lemmaID = "";

                    if ((abstract_id = getID("category", abstract_category,
                                             QString("type=%1 AND abstract=1").arg((int)types[j]))) == -1) {
                        abstract_id = insert_category(abstract_category, types[j], source_id, true);
                        theSarf->out << QString("Inserted new Abstract Category '%1'\n").arg(abstract_category);
                    }
                }
            }

            QList<long> *abstract_categories = new QList<long>();
            abstract_categories->append(abstract_id);

            if (insert_item(types[j], item, raw_data, category,
                            source_id, abstract_categories,
                            description, POS, "", lemmaID) < 0) {
                theSarf->out << "Error at line " << line_num << ": '" << line
                             << "', Item was not inserted\n";
                return -1;
            }
        }

        theSarf->out <<
                     QString("\nSuccessfully processed all %1 %2 entries\n")
                     .arg(line_num)
                     .arg(interpret_type(types[j]));
        input.close();
    }

    // compatibility rules
    const int num_files_rules = 5;
    const QString rules_files[num_files_rules] = {
        folder + "tableAA" + tag,
        folder + "tableCC" + tag,
        folder + "tableAB",
        folder + "tableBC",
        folder + "tableAC"
    };
    const rules rule[num_files_rules] = {AA, CC, AB, BC, AC};

    for (int j = 0; j < num_files_rules; j++) {
        QFile input(rules_files[j]);

        if (!input.open(QIODevice::ReadWrite)) {
            theSarf->out << QString("File %1 not found\n").arg(rules_files[j]);

            if (j < 2) {
                continue;
            } else {
                return 1;
            }
        }

        QTextStream file(&input);
        file.setCodec("utf-8");
        int line_num = 0;

        while (!file.atEnd()) {
            line_num++;
            QString line = file.readLine(0);

            if (line.isNull()) {
                line_num--; // finished
                break;
            }

            // ignore empty lines and comments if they exist
            if (line.isEmpty() || line.startsWith(";")) {
                continue;
            }

            QStringList entries;

            if (rule[j] == AA || rule[j] == CC) {
                entries = line.split("\t", QString::KeepEmptyParts);
            } else {
                entries = line.split(QRegExp(QString("[\t ]")), QString::KeepEmptyParts);
            }

            if (entries.size() < 2) {
                theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
                return -1;
            }

            QString cat1 = entries[0];
            QString cat2 = entries[1];
            QString resCat = cat2;
            QString inflectionRule;

            if (rule[j] == AA || rule[j] == CC) {
                item_types type = (rule[j] == AA ? PREFIX : SUFFIX);

                if (entries.size() < 3) {
                    theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
                    return -1;
                } else {
                    resCat = entries[2];

                    if (entries.size() > 3) {
                        for (int i = 3; i < entries.size(); i++) {
                            inflectionRule += entries[i] + " ";
                        }
                    }

                    bool exp1 = cat1.startsWith("{") && cat1.endsWith("}"),
                         exp2 = cat2.startsWith("{") && cat2.endsWith("}"),
                         exp3 = resCat.startsWith("{") && resCat.endsWith("}");

                    if (exp1 || exp2 || exp3) {
                        if (insertRuleAccordingToExpression(type, cat1, cat2, resCat, inflectionRule, source_id) < 0) {
                            theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
                        }

                        continue; // it will call insert_compatibility_rules as needed internally (maybe multiple times)
                    } else {
                        assert(insert_category(cat1, (rule[j] == AA ? PREFIX : SUFFIX), source_id, false) >= 0);
                        assert(insert_category(cat2, (rule[j] == AA ? PREFIX : SUFFIX), source_id, false) >= 0);
                        assert(insert_category(resCat, (rule[j] == AA ? PREFIX : SUFFIX), source_id, false) >= 0);
                    }
                }
            } else {
                // if not AB or AC => BC
                assert(insert_category(cat1, (rule[j] == AB || rule[j] == AC ? PREFIX : STEM), source_id, false) >= 0);
                //if not AC or BC => AB
                assert(insert_category(cat2, (rule[j] == AC || rule[j] == BC ? SUFFIX : STEM), source_id, false) >= 0);
            }

            if (insert_compatibility_rules(rule[j], cat1, cat2, resCat, inflectionRule, source_id) < 0) {
                theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
                continue; // files contain undefined categories, so just ignore these
            }
        }

        theSarf->out <<
                     QString("\nSuccessfully processed all %1 %2 compatibility rules\n")
                     .arg(line_num)
                     .arg(interpret_type(rule[j]));
        input.close();
    }

    return 0;
}

int insert_propernames() {
    QDir folder("../../dic/N", "*.txt");

    if (!folder.exists()) {
        theSarf->out << "Invalid Folder\n";
        return -1;
    }

    // returns id if already present
    long abstract_Noun_Prop_id = insert_category("NOUN_PROP", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_people_names = insert_category("Name of Person", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_male_names = insert_category("Male Names", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_female_names = insert_category("Female Names", STEM, dbitvec(max_sources), true);
    QString file_name;
    long abstract_category_id;
    foreach(file_name, folder.entryList()) {
        QString ab = file_name.split(".").at(0);

        if (file_name == "all_n.txt") {
            theSarf->out << "Ignored " + file_name + " file\n";
            continue;
        }

        QFile input(folder.absolutePath().append(QString("/").append(file_name)));

        if (!input.open(QIODevice::ReadWrite)) {
            theSarf->out << "Unexpected Error: File not found\n";
            return 1;
        }

        QTextStream file(&input);
        file.setCodec("utf-8");
        int line_num = 0;
        int source_id;
        QString source;
        QString normalization_process;

        while (!file.atEnd()) {
            line_num++;
            QString line = file.readLine(0);

            if (line_num == 1) {
                source = line;
                continue;
            } else if (line_num == 2) {
                normalization_process = line;
                continue;
            } else if (line_num == 3) {
                source_id = insert_source(source, normalization_process, line);
                abstract_category_id = insert_category(ab, STEM, source_id, true);
                continue;
            }

            // finished
            if (line.isNull()) {
                line_num--;
                break;
            }

            // ignore empty lines if they exist
            if (line.isEmpty()) {
                continue;
            }

            QList<long> *abstract_categories = new QList<long>();

            if (ab != "eNarrator Names") {
                abstract_categories->append(abstract_Noun_Prop_id);
            }

            abstract_categories->append(abstract_category_id);
            abstract_categories->append(abstract_people_names);
            QStringList entries = line.split("\t");
            QString name = entries[0];

            if (ab.contains("Hebrew")) {
                if (entries[1] == "M") {
                    abstract_categories->append(abstract_male_names);
                } else {
                    abstract_categories->append(abstract_female_names);
                }
            }

            if (insert_NProp(name, abstract_categories, source_id, "Name of Person") < 0) {
                theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
                return -1;
            }

            delete abstract_categories;
        }

        theSarf->out << QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(file_name);
        input.close();
    }
    return 0;
}

int insert_placenames() {
    // not yet complete
    QDir folder("../../dic/P");

    if (!folder.exists()) {
        theSarf->out << "Invalid Folder\n";
        return -1;
    }

    int folders_source_id = insert_source("ar.wikipedia.org/",
                                          "direct copy from html and dividing them into folders "
                                          "and subfiles according to continents and cities/towns",
                                          "Hamza Harkous");
    // returns id if already present
    long abstract_Noun_Prop_id = insert_category("NOUN_PROP", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_place_names = insert_category("Name of Place", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_continent_name = insert_category("Continent", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_country_name = insert_category("Country", STEM, dbitvec(max_sources), true);
    // returns id if already present
    long abstract_city_name = insert_category("City/Town", STEM, dbitvec(max_sources), true);
    QString file_name;
    QStringList h = folder.entryList();
    foreach(file_name, folder.entryList()) {
        if (file_name.endsWith(".txt")) {
            if (file_name.startsWith("Google")) {
                theSarf->out << QString("Ignored %1 file\n").arg(file_name);
                continue;
            }

            long abstract_category_id;
            QFile input(folder.absolutePath().append(QString("/").append(file_name)));

            if (!input.open(QIODevice::ReadWrite)) {
                theSarf->out << "Unexpected Error: File not found\n";
                return 1;
            }

            QTextStream file(&input);
            file.setCodec("utf-8");
            int line_num = 0;
            int source_id;
            QString source;
            QString normalization_process;

            while (!file.atEnd()) {
                line_num++;
                QString line = file.readLine(0);

                if (line_num == 1) {
                    source = line;
                    continue;
                } else if (line_num == 2) {
                    normalization_process = line;
                    continue;
                } else if (line_num == 3) {
                    source_id = insert_source(source, normalization_process, line);
                    abstract_category_id = insert_category(file_name.split(".").at(0), STEM, source_id, true);
                    continue;
                }

                // finished
                if (line.isNull()) {
                    line_num--;
                    break;
                }

                // ignore empty lines if they exist
                if (line.isEmpty()) {
                    continue;
                }

                QList<long> *abstract_categories = new QList<long>();
                abstract_categories->append(abstract_Noun_Prop_id);
                abstract_categories->append(abstract_category_id);
                abstract_categories->append(abstract_place_names);
                insert_NProp(line, abstract_categories, source_id, file_name.split(".").at(0));
            }

            theSarf->out <<
                         QString("\nSuccessfully processed all %1 %2 entries\n")
                         .arg(line_num)
                         .arg(file_name);
            input.close();
        } else if (!file_name.contains('.')) {
            QDir folder2(folder.absolutePath().append(QString("/").append(file_name)), "*.txt");

            if (!folder2.exists()) {
                theSarf->out << "Unexpected Error: Folder that was assumed to exist does not\n";
                return -1;
            }

            QStringList continent = file_name.split("-");
            QString continent_english = continent[0];

            if (continent.count() > 1) {
                QString continent_arabic = continent[1];
                QList<long> *abstract_categories = new QList<long>();
                abstract_categories->clear();
                abstract_categories->append(abstract_Noun_Prop_id);
                abstract_categories->append(abstract_continent_name);
                abstract_categories->append(abstract_place_names);
                insert_NProp(continent_arabic, abstract_categories, folders_source_id, continent_english);
            }

            dbitvec sources(max_sources);
            sources.reset();
            long continent_id = insert_category(continent_english, STEM, folders_source_id, true);
            QString file_name2;
            foreach(file_name2, folder2.entryList()) {
                QFile input(folder2.absolutePath().append(QString("/").append(file_name2)));

                if (!input.open(QIODevice::ReadWrite)) {
                    theSarf->out << "Unexpected Error: File not found\n";
                    return 1;
                }

                QTextStream file(&input);
                file.setCodec("utf-8");
                int line_num = 0;
                QStringList country = file_name2.split("-");
                QString country_english = country[0];

                if (country.count() > 1) {
                    QString country_arabic = country[1].split(".")[0];
                    QList<long> *abstract_categories = new QList<long>();
                    abstract_categories->append(abstract_Noun_Prop_id);
                    abstract_categories->append(abstract_country_name);
                    abstract_categories->append(abstract_place_names);
                    abstract_categories->append(continent_id);
                    insert_NProp(country_arabic, abstract_categories, folders_source_id, country_english);
                }

                long country_id = insert_category(country_english, STEM, folders_source_id, true);

                while (!file.atEnd()) {
                    line_num++;
                    QString line = file.readLine(0);

                    // finished
                    if (line.isNull()) {
                        line_num--;
                        break;
                    }

                    // ignore empty lines if they exist
                    if (line.isEmpty()) {
                        continue;
                    }

                    // ignore lines that are just for sorting in alphabatical order
                    if (line.split(" ")[0].length() == 1) {
                        continue;
                    }

                    QList<long> *abstract_categories = new QList<long>();
                    abstract_categories->append(abstract_Noun_Prop_id);
                    abstract_categories->append(abstract_city_name);
                    abstract_categories->append(abstract_place_names);
                    abstract_categories->append(continent_id);
                    abstract_categories->append(country_id);
                    QString alltext = line.split('(')[0];
                    QStringList city = alltext.split(QRegExp("[.-,]"));

                    // add all the text in addition to the primary part
                    if (city.count() > 1) {
                        insert_NProp(alltext, abstract_categories, folders_source_id, "city/town in " + country_english);
                    }

                    insert_NProp(city[0], abstract_categories, folders_source_id, "city/town in " + country_english);
                }

                theSarf->out <<
                             QString("\nSuccessfully processed all %1 %2 entries\n")
                             .arg(line_num)
                             .arg(file_name2);
                input.close();
            }
        }
    }
    return 0;
}

int insert_time_categorizations() {
    int source_id = insert_source("Jad Time Categories", "Manual Work", "Jad Makhlouta");
    QString file_name = "../../dic/T/categorized.txt";
    QFile input(file_name);

    if (!input.open(QIODevice::ReadWrite)) {
        theSarf->out << "Unexpected Error: File not found\n";
        return 1;
    }

    QTextStream file(&input);
    file.setCodec("utf-8");
    int line_num = 0;

    while (!file.atEnd()) {
        line_num++;
        QString line = file.readLine(0);

        // finished
        if (line.isNull()) {
            line_num--;
            break;
        }

        // ignore empty lines if they exist
        if (line.isEmpty()) {
            continue;
        }

        QList<long> *abstract_categories = new QList<long>();
        // returns id if already present
        long time_abstract_category_id = insert_category("Time", STEM, source_id, true);
        abstract_categories->append(time_abstract_category_id);
        QStringList entries = line.split("\t", QString::KeepEmptyParts);

        if (entries.size() < 7) {
            theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
            return -1;
        }

        QString item = entries[0];
        QString raw_data = entries[1];
        QString category = entries[2];
        QString description = entries[3];
        QString POS = entries[4];

        // 3 letters are not properly transfered using perl script because it is
        // not based on unicode, here we solve them manually
        if (item.contains("{") || item.contains("`") || item.contains("V")) {
            item = item.replace("{", QString(alef_wasla));
            item = item.replace("`", QString(aleft_superscript));
            item = item.replace("V", QString(veh));
        }

        if (raw_data.contains("{") || raw_data.contains("`") || raw_data.contains("V")) {
            raw_data = raw_data.replace("{", QString(alef_wasla));
            raw_data = raw_data.replace("`", QString(aleft_superscript));
            raw_data = raw_data.replace("V", QString(veh));
        }

        // lemma is 5
        QString cateorized_abstract_category = entries[6];
        // returns id if already present
        long cateorized_abstract_category_id = insert_category(cateorized_abstract_category, STEM, source_id, true);
        abstract_categories->append(cateorized_abstract_category_id);

        if (!addAbstractCategory(item, raw_data, category, source_id, abstract_categories, description, POS)) {
            theSarf->out << "Error at line " << line_num << ": '" << line << "'\n";
        }
    }

    theSarf->out <<
                 QString("\nSuccessfully processed all %1 %2 entries\n")
                 .arg(line_num)
                 .arg(file_name);
    input.close();
    return 0;
}
