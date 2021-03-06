#include "hadithCommon.h"
#include "narrator_abstraction.h"
#include <QStringList>
#include <QString>
#include <assert.h>
#include "Triplet.h"
#include "stemmer.h"
#include <QPair>
#include "hadith_utilities.h"
#include "narratorHash.h"
#include "graph.h"
#include "biographyGraphUtilities.h"

#define display(c) ;

qint8 getType(const NarratorPrim *) {
    return 'n';
}
qint8 getType(const NamePrim *) {
    return 'N';
}
qint8 getType(const ChainPrim *) {
    return 'C';
}
qint8 getType(const NameConnectorPrim *) {
    return 'c';
}
qint8 getType(const Narrator *) {
    return 'a';
}
qint8 getType(const NarratorConnectorPrim *) {
    return 'p';
}
qint8 getType(const Chain *) {
    return 'A';
}

NarratorPrim::NarratorPrim(QString *hadith_text) {
    m_start = 0;
    m_end = 0;
    this->hadith_text = hadith_text;
}

NarratorPrim::NarratorPrim(QString *hadith_text, int m_start) {
    this->m_start = m_start;
    this->hadith_text = hadith_text;
}

Narrator::Narrator(QString *hadith_text): ChainPrim(hadith_text) {
    isRasoul = false;
}
bool Narrator::isNarrator() const {
    return true;
}
void Narrator::serialize(QTextStream &chainOut) const {
    chainOut << "\tNarrator {\n";
    int size = m_narrator.size();

    for (int i = 0; i < size; i++) {
        chainOut << "\t";
        m_narrator[i]->serialize(chainOut);
        //chainOut<<m_narrator[i]->m_start<<" "<<m_narrator[i]->m_end<<"";
        chainOut << "\t";
        chainOut << hadith_text->mid(m_narrator[i]->m_start, m_narrator[i]->m_end - m_narrator[i]->m_start + 1) << " ";
        chainOut << "\n";
    }

    chainOut << "\t\t}\n";
}
void Narrator::serialize(QDataStream &chainOut) const {
    chainOut << getType((const Narrator *)this);
    chainOut << isRasoul;
    int size = m_narrator.size();
    //assert(size>0);
    chainOut << (qint32)size;

    for (int i = 0; i < size; i++) {
        m_narrator[i]->serialize(chainOut);
        chainOut << m_narrator[i]->m_start << m_narrator[i]->m_end;
    }
}

void Narrator::deserialize(QDataStream &chainIn) {
    /*qint8 c;
      chainIn>>c;
      assert(c==getType((const Narrator*)this));*/
    chainIn >> isRasoul;
    qint32 size;
    chainIn >> size;

    for (int i = 0; i < size; i++) {
        NarratorPrim *narrator_prim = NULL;
        qint8 c;
        chainIn >> c;

        if (c == getType((const NameConnectorPrim *)narrator_prim)) {
            narrator_prim = new NameConnectorPrim(hadith_text);
        } else if (c == getType((const NamePrim *)narrator_prim)) {
            narrator_prim = new NamePrim(hadith_text);
        } else {
            throw "stop";
        }

        narrator_prim->deserialize(chainIn);
        chainIn >> narrator_prim->m_start >> narrator_prim->m_end;
        m_narrator.append(narrator_prim);
    }
}

NamePrim::NamePrim(QString *hadith_text): NarratorPrim(hadith_text) {
    learnedName = false;
}
NamePrim::NamePrim(QString *hadith_text, int m_start): NarratorPrim(hadith_text, m_start) {
    learnedName = false;
}

bool NamePrim::isNamePrim() const {
    return true;
}
void NamePrim::serialize(QTextStream &chainOut) const {
    chainOut << "\tName ";
}
void NamePrim::serialize(QDataStream &chainOut) const {
    chainOut << getType((const NamePrim *)this);
    chainOut << learnedName;
}
void NamePrim::deserialize(QDataStream &chainIn) {
    /*qint8 c;
      chainIn>>c;
      assert(c==getType((const NamePrim*)this));*/
    chainIn >> learnedName;
}

NameConnectorPrim::NameConnectorPrim(QString *hadith_text): NarratorPrim(hadith_text), type(OTHER) {}
NameConnectorPrim::NameConnectorPrim(QString *hadith_text, int m_start): NarratorPrim(hadith_text, m_start),
    type(OTHER) {}

bool NameConnectorPrim::isNamePrim() const {
    return false;
}
void NameConnectorPrim::serialize(QTextStream &chainOut) const {
    chainOut << "\t\tName Conn. ";
}
void NameConnectorPrim::serialize(QDataStream &chainOut) const {
    chainOut << getType((const NameConnectorPrim *)this);
    chainOut << (qint64)m_start << (qint64)m_end << (quint8)type;
}
void NameConnectorPrim::deserialize(QDataStream &chainIn) {
    /*qint8 c;
      chainIn>>c;
      assert(c==getType((const NameConnectorPrim*)this));*/
    qint64 start, end;
    quint8 l_type;
    chainIn >> start >> end >> l_type;
    m_start = start;
    m_end = end;
    type = (Type)l_type;
}

NarratorConnectorPrim::NarratorConnectorPrim(QString *hadith_text): ChainPrim(hadith_text) {
    m_start = 0;
    m_end = 0;
    this->hadith_text = hadith_text;
}

NarratorConnectorPrim::NarratorConnectorPrim(QString *hadith_text, int m_start): ChainPrim(hadith_text) {
    this->m_start = m_start;
}

bool NarratorConnectorPrim::isNarrator() const {
    return false;
}
void NarratorConnectorPrim::serialize(QTextStream &chainOut) const {
    chainOut << "\tNarrator Connector: "; //<<m_start<<" "<<m_end<<"";
    chainOut << "\t\t";
    chainOut << getString() << " ";
    chainOut << "\n";
}
void NarratorConnectorPrim::serialize(QDataStream &chainOut) const {
    chainOut << getType((const NarratorConnectorPrim *)this);
    chainOut << (qint64)m_start << (qint64)m_end;
}
void NarratorConnectorPrim::deserialize(QDataStream &chainIn) {
    /*qint8 c;
      chainIn>>c;
      assert(c==getType((const NarratorConnectorPrim*)this));*/
    qint64 start, end;
    chainIn >> start >> end;
    m_start = start;
    m_end = end;
}
Chain::Chain(QString *hadith_text) {
    this->hadith_text = hadith_text;
}
void Chain::serialize(QTextStream &chainOut) const {
    int size = m_chain.size();
    chainOut << " CH {\n";

    for (int i = 0; i < size; i++) {
        m_chain[i]->serialize(chainOut);
    }

    chainOut << "}\n\n";
}
void Chain::serialize(QDataStream &stream) const {
    stream << getType((const Chain *)this);
    int size = m_chain.size();
    stream << (qint32)size;

    for (int i = 0; i < size; i++) {
        m_chain[i]->serialize(stream);
    }
}
void Chain::deserialize(QDataStream &stream) {
    qint8 c;
    stream >> c;
    assert(c == getType((const Chain *)this));
    qint32 size;
    stream >> size;

    for (int i = 0; i < size; i++) {
        ChainPrim *chain_prim = NULL;
        qint8 c;
        stream >> c;

        if (c == getType((const NarratorConnectorPrim *)chain_prim)) {
            chain_prim = new NarratorConnectorPrim(hadith_text);
        } else if (c == getType((const Narrator *)chain_prim)) {
            chain_prim = new Narrator(hadith_text);
        } else {
            throw "stop";
        }

        chain_prim->deserialize(stream);
        m_chain.append(chain_prim);
    }
}
ChainPrim::ChainPrim(QString *hadith_text) {
    this->hadith_text = hadith_text;
}
/*
   void ChainPrim::serialize(QDataStream & chainOut) const
   {
   chainOut<<getType((const ChainPrim*)this);
   }
   void ChainPrim::deserialize(QDataStream & chainIn)
   {
   qint8 c;
   chainIn>>c;
   assert(c==getType((const ChainPrim*)this));
   }
   void ChainPrim::serialize(QTextStream &) const {}*/


QDataStream &operator>>(QDataStream &in, ChainNarratorPrim &p) {
    p.deserialize(in);
    return in;
}
QDataStream &operator<<(QDataStream &out, const ChainNarratorPrim &p) {
    p.serialize(out);
    return out;
}

typedef QPair<int, long> ItemIDCategory;
typedef QVector<ItemIDCategory> StemList;
class StemsDetector: public Stemmer {
    private:
        int forced_finish_pos;
    public:
        StemList stems;

        StemsDetector(QString *text, int start, int finish): Stemmer(text, start, false) {
            stems.clear();
            forced_finish_pos = finish;
            multi_p.abstract_category = false;
            multi_p.POS = false;
            multi_p.description = false;
            multi_p.raw_data = false;
            setSolutionSettings(multi_p);
        }
        bool on_match() {
            if (info.finish == forced_finish_pos) { //TODO: check if is Name or save initially all stems without re-stemming
                ItemIDCategory result(Stem->id_of_currentmatch, Stem->category_of_currentmatch);
                stems.append(result);
            }

            return true;
        }
};

inline bool has_equal_stems(StemList l1, StemList l2) {
    for (int i = 0; i < l1.count(); i++) {
        for (int j = 0; j < l2.count(); j++) {
            //qDebug()<<l1[i]<<"-"<<l2[j];
            if (l1[i] == l2[j]) {
                return true;
            }
        }
    }

    return false;
}

inline bool has_common_element(QList<QString> l1, QList<QString> l2) {
    for (int i = 0; i < l1.count(); i++) {
        for (int j = 0; j < l2.count(); j++) {
            //qDebug()<<l1[i]<<"-"<<l2[j];
            if (l1[i] == l2[j]) {
                return true;
            }
        }
    }

    return false;
}

class IbnStemsDetector: public Stemmer {
    private:
        int forced_finish_pos;
    public:
        bool ibn, place, city, country;
        QList<QString> country_names, city_names;
        StemList stems;

        IbnStemsDetector(QString *text, int start, int finish): Stemmer(text, start, false) {
            stems.clear();
            forced_finish_pos = finish;
            multi_p.abstract_category = true;
            multi_p.POS = true;
            multi_p.description = true;
            multi_p.raw_data = true;
            ibn = false;
            city = false;
            place = false;
            country = false;
            setSolutionSettings(multi_p);
        }
        bool on_match() {
            if (info.finish == forced_finish_pos) {
                ItemIDCategory result(Stem->id_of_currentmatch, Stem->category_of_currentmatch);
                stems.append(result);

                if (!ibn) {
                    solution_position *stem_sol = Stem->computeFirstSolution();

                    do {
                        if (IBN_descriptions.contains(Stem->solution->description_id())) {
                            ibn = true;
                            break;
                        } else if (Stem->solution->abstract_categories.getBit(bit_PLACE)) {
                            place = true;

                            //display (Stem->solution->raw_data +":"+Stem->solution->description().append("\n"));
                            if (Stem->solution->abstract_categories.getBit(bit_CITY)) {
                                city = true;
                                //display("{city}\n");
                                city_names.append(Stem->solution->raw_data);
                                country_names.append(Stem->solution->description().split("city/town in ").at(1));
                            }

                            if (Stem->solution->abstract_categories.getBit(bit_COUNTRY)) {
                                //display("{country}\n");
                                country = true;
                                QString desc = Stem->solution->description();

                                if (Stem->solution->abstract_categories.getBit(bit_POSSESSIVE)) {
                                    country_names.append(desc.split("possessive form of ").at(1));
                                } else {
                                    country_names.append(desc);
                                }
                            }

                            break;
                        }
                    } while (Stem->computeNextSolution(stem_sol));

                    delete stem_sol;
                }
            }

            return true;
        }
};


class Int2 {
    public:
        int first: 16;
        int second: 16;
        Int2(int f, int s): first(f), second(s) { }
        int difference() {
            int d = first - second;
            return (d >= 0 ? d : -d);
        }
};
typedef Triplet<NamePrim, NamePrim, Int2> EqualNamesStruct;
typedef Triplet<NameConnectorPrim, NameConnectorPrim, int> EqualConnsStruct;

inline double getdistance(const Narrator &n1, const Narrator &n2) {
    double max_distance = hadithParameters.equality_threshold * 2;
    bool abihi1 = isRelativeNarrator(n1), abihi2 = isRelativeNarrator(n2);

    if (abihi1 || abihi2) {
        return max_distance;    //we dont know yet to what person is the ha2 in abihi a reference so they might not be equal.
    }

    QString n1_str = n1.getString(), n2_str = n2.getString();

    if (equal(n1_str, n2_str)) {
        return 0;
    }

    bool ras1 = n1.isRasoul, ras2 = n2.isRasoul;

    if (ras1 && ras2) {
        return 0;
    } else if (ras1 || ras2) {
        return max_distance;
    }

    double dist = max_distance, delta = hadithParameters.equality_delta;
    QList<NamePrim > Names1, Names2;
    QList<NameConnectorPrim > Conns1, Conns2;
    bool first_ibn1 = false, first_ibn2 = false; //means that the corresponding chain starts with ibn before any name

    for (int i = 0; i < n1.m_narrator.count(); i++)
        if (n1.m_narrator[i]->isNamePrim()) {
            Names1.append(*(NamePrim *)n1.m_narrator[i]);
        } else {
            NameConnectorPrim &c = *(NameConnectorPrim *)n1.m_narrator[i];
            Conns1.append(c);

            if (Names1.size() == 0) {
                if (c.isIbn()) {
                    first_ibn1 = true;
                }
            }
        }

    for (int i = 0; i < n2.m_narrator.count(); i++)
        if (n2.m_narrator[i]->isNamePrim()) {
            Names2.append(*(NamePrim *)n2.m_narrator[i]);
        } else {
            NameConnectorPrim &c = *(NameConnectorPrim *)n2.m_narrator[i];
            Conns2.append(c);

            if (Names2.size() == 0) {
                if (c.isIbn()) {
                    first_ibn2 = true;
                }
            }
        }

    QList<EqualNamesStruct> equal_names;

    for (int i = 0; i < Names1.count(); i++) {
        for (int j = 0; j < Names2.count(); j++) {
            if (equal_ignore_diacritics(Names1[i].getString(), Names2[j].getString())) {
                equal_names.append(EqualNamesStruct(Names1[i], Names2[j], Int2(i, j)));
            }
        }
    }

    if (equal_names.count() == 0) {
        return dist;
    }

    if (equal_names.count() == min(Names1.count(), Names2.count())) {
        display(QString("%1,%2,%3").arg(equal_names.count()).arg(Names1.count()).arg(Names2.count()));
        display("equal names \\ ");
        dist -= delta;
    }

    int number_skipped_names = equal_names[0].third.difference(); //start by number of skipped items

    for (int i = 1; i < equal_names.count(); i++) {
        int diff = equal_names[i].third.difference();

        if (diff != number_skipped_names) {
            number_skipped_names = -1;
        }

        if (equal_names[i - 1].third.second > equal_names[i].third.second) {
            return dist;
        }
    }

    display("same order of names \\ ");
    dist -= delta; //if reaches here it means names are in correct order, otherwise it would have returned before
    QList<EqualConnsStruct> equal_conns;
    int num_ibn1 = 0, num_ibn2 = 0;

    for (int i = 0; i < Conns1.count(); i++) {
        IbnStemsDetector stemsD1(Conns1[i].hadith_text, Conns1[i].m_start, Conns1[i].m_end);
        stemsD1();

        if (stemsD1.ibn) {
            Conns1.removeAt(i);
            num_ibn1++;
            i--;
        }

        for (int j = 0; j < Conns2.count(); j++) {
            IbnStemsDetector stemsD2(Conns2[j].hadith_text, Conns2[j].m_start,
                                     Conns2[j].m_end); //TODO: later for efficiency, perform in a seperate loop, save them then use them
            stemsD2();

            if (stemsD2.ibn) {
                Conns2.removeAt(j);
                num_ibn2++;
                j--;
            }

            //TODO: here check if mutually exclusive and if so, punish
            if (stemsD1.city && stemsD2.city) {
                if (!has_common_element(stemsD1.country_names, stemsD2.country_names)) {
                    display("cities located in different countries \\ ");
                    dist += delta;
                } else {
                    if (!has_common_element(stemsD1.city_names, stemsD2.city_names)) {
                        display("different cities located in same country \\ ");
                        dist += delta;
                    } else {
                        display("same city \\ ");
                        dist -= delta;
                    }
                }

                continue;
            } else if ((stemsD1.city && stemsD2.country) || (stemsD2.city && stemsD1.country)) {
                if (!has_common_element(stemsD1.country_names, stemsD2.country_names)) {
                    display(QString("city and country mutually exclusive \\")); // %1 VS %2\\ ").arg(stemsD1.country_names[0]).arg(stemsD2.country_names[0]));
                    dist += delta;
                } else {
                    display("city contained in country \\ ");
                    dist -= delta / 2;
                }

                continue;
            } else if (stemsD2.country && stemsD1.country) {
                if (!has_common_element(stemsD1.country_names, stemsD2.country_names)) {
                    display("different countries \\ ");
                    dist += delta;
                } else {
                    display("same countries \\ ");
                    dist -= delta;
                }

                continue;
            }

            //TODO: maybe punish more and also add more different rules other than places
            if (has_equal_stems(stemsD1.stems, stemsD2.stems) && !stemsD1.ibn &&
                !stemsD2.ibn) { //ibn checks redundant removed before
                equal_conns.append(EqualConnsStruct(Conns1[i], Conns2[j], j));
            }

            //TODO: ibn must have a seperate rule which is inter-related to names
        }
    }

    int min_names = min(Names1.count(), Names2.count()),
        min_ibn_cnt = min(num_ibn1, num_ibn2);
    bool one_ibn_first = ((!first_ibn1 && first_ibn2) || (!first_ibn2 && first_ibn1));
    bool correctly_skipped = one_ibn_first && ((first_ibn1 &&  num_ibn1 == min_ibn_cnt && Names1.count() == min_names) ||
                                               (first_ibn2 &&  num_ibn2 == min_ibn_cnt && Names2.count() == min_names));

    if ((min_ibn_cnt + 1 == min_names) || (one_ibn_first && min_ibn_cnt == min_names && correctly_skipped)) {
        if (number_skipped_names == 0 && !one_ibn_first) {
            display("all names are there\\ ");//if all detected names are correct
            dist -= delta; //reward when all names are there
        }

        if (number_skipped_names == 1 && one_ibn_first) {
            display("all names are there except the first and skipped by ibn \\ ");
            dist -= delta / 2; //reward when starts by ibn, i.e. one name of the list is skipped
        }
    } else {
        //revert wrong decision previously if needed
        if (equal_names.count() == min(Names1.count(), Names2.count())) {
            display("-{equal names} \\ ");//some names are not detected and from the number of ibn, we deduce they are not equal
            dist += delta;
        }
    }

    if (Conns1.count() == 0 || Conns2.count() == 0) {
        display("No connectors \\ ");
        dist -= delta;
    }

    if (equal_conns.count() > 0) {
        display(QString("%1 identical connectors :{ ").arg(equal_conns.count()));
        dist -= delta * equal_conns.count(); //reward as much as there are identical connectors other than ibn
    }

    display("\n");
    return min(max(dist - equal_conns.count() * delta / 2, 0.0), max_distance);
}



inline double equalNew(const Narrator &n1, const Narrator &n2) {
    bool abihi1 = isRelativeNarrator(n1), abihi2 = isRelativeNarrator(n2);

    if (abihi1 || abihi2) {
        return 0;    //we dont know yet to what person is the ha2 in abihi a reference so they might not be equal.
    }

    if (n1.isRasoul || n2.isRasoul) {
        if (n1.isRasoul && n2.isRasoul) {
            return 1;
        } else {
            return 0;
        }
    }

    QString n1_str = n1.getString(), n2_str = n2.getString();

    if (n1_str == n2_str) {
        return 1;
    }

    Narrator::NarratorPrimHierarchy names1, names2;
    Narrator::PossessiveList possessives1, possessives2;
    n1.preProcessForEquality(names1, possessives1);
    n2.preProcessForEquality(names2, possessives2);
    int levelMin = min(names1.size(), names2.size());
    int levelMax = max(names1.size(), names2.size());
    int levelsEqual = 0;

    for (int i = 0; i < levelMin; i++) {
        int namesMin = min(names1[i].size(), names2[i].size());
        int equal = false;

        for (int j = 0; j < namesMin; j++) {
            if (!names1[i][j]->isNamePrim() && !names2[i][j]->isNamePrim()) {
                continue;
            }

            QString name1 = names1[i][j]->getString(),
                    name2 = names2[i][j]->getString();

            if (name1.size() > 0 && alefs.contains(name1[0])) {
                name1[0] = alef;
            }

            if (name2.size() > 0 && alefs.contains(name2[0])) {
                name2[0] = alef;
            }

            if (name1 != name2) { //TODO: check if this works when diacritics are available???
                return 0;
            } else {
                equal = true;
            }
        }

        if (names1[i].size() == names2[i].size() && names1[i].size() == 0) {
            equal = true;
        }

        if (equal) {
            levelsEqual++;
        }
    }

    //till now possessives not even compared
    return (double)levelsEqual / levelMax;
}


double equal(const Narrator &n1, const  Narrator   &n2) {
    return equalNew(n1, n2);
}

double Narrator::equals(const Narrator &rhs) const {
    return equal(*this, rhs);
}

QString Narrator::getKey() const {
    bool abihi = isRelativeNarrator(*this);

    if (abihi) {
        return abyi + ha2;    //we dont know yet to what person is the ha2 in abihi a reference so they might not be equal.
    }

    if (isRasoul) {
        return alrasoul;
    }

    NarratorPrimHierarchy names;
    PossessiveList possessives;
    preProcessForEquality(names, possessives);
    return NarratorHash::getKey(names, possessives, names.size(), true, false);
}

void Biography::serialize(QDataStream &chainOut) const {
    int size = narrators.size();
    chainOut << start << end << size;

    for (int i = 0; i < narrators.size(); i++) {
        narrators[i]->narrator->serialize(chainOut);
        chainOut << narrators[i]->isRealNarrator;
        chainOut << nodeGroups.size();

        for (int j = 0; j < nodeGroups.size(); j++) {
            int size = nodeGroups[j].size();
            chainOut << size;

            for (int k = 0; k < size; k++) {
                const MatchingNode &m = nodeGroups[j][k];
                chainOut << m.node->getId() << m.similarity;
            }
        }

    }
}
void Biography::deserialize(QDataStream &chainIn) {
    int size;
    chainIn >> start >> end >> size;

    for (int i = 0; i < size; i++) {
        Narrator *n = new Narrator(text);
        qint8 c;
        chainIn >> c;
        assert(c == getType(n));
        n->deserialize(chainIn);
        BiographyNarrator *bio_narr = new BiographyNarrator(n);
        chainIn >> bio_narr->isRealNarrator;
        narrators.append(bio_narr);
        int size;
        chainIn >> size;

        for (int j = 0; j < size; j++) {
            int size;
            chainIn >> size;
            nodeGroups.append(NarratorNodeList());

            for (int k = 0; k < size; k++) {
                int cInt;
                double similarity;
                chainIn >> cInt
                        >> similarity;
                NarratorNodeIfc *n = graph->getNode(cInt);
                assert(n != NULL);
                nodeGroups[j].append(MatchingNode(n, similarity));
            }
        }

    }
}
void Biography::serialize(QTextStream &chainOut) const {
    for (int i = 0; i < narrators.size(); i++) {
        chainOut << i << ": " << (narrators[i]->isRealNarrator ? "Real Narrator" : "");
        narrators[i]->narrator->serialize(chainOut);
        chainOut << "\n";
    }
}

bool Biography::isRealNarrator(Narrator *n) {
    int i;

    if (!nodeGroups.isEmpty()) {
        if (nodeGroups.last().size() > 0) {
            nodeGroups.append(NarratorNodeList());
        }

        i = nodeGroups.size() - 1;
    } else {
        nodeGroups.append(NarratorNodeList());
        i = 0;
    }

    if (n->isRasoul) {
        return false;
    } else {
        bool isReal =::isRealNarrator(graph, n, nodeGroups[i]);

        if (!isReal) {
            nodeGroups.removeLast();
            return false;
        } else {
            return true;
        }
    }
}


