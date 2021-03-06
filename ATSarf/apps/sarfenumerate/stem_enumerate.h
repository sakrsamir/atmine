#ifndef STEM_ENUMERATE_H
#define STEM_ENUMERATE_H

#include "atmTrie.h"
#include "common.h"
#include "database_info_block.h"
#include "solution_position.h"
#include <QString>

class StemEnumerate {
    public:
        /// Specifies the category of the possible prefix extracted
        long prefix_category;
        /// Holds the category of the stem possible match
        long category_of_currentmatch;
        /// Holds the id of the node stem possible match
        long id_of_currentmatch;
        /// This structure holds the information of the possible stem solution found
        minimal_item_info *solution;

    private:
        /// Delcares a pointer to the trie structure that stores the root arabic words in the lexer
        ATTrie *trie;
        multiply_params multi_p;
        bool stop;
        QVector<QString> possible_raw_datas;

    public:
        StemEnumerate(long prefix_category);
        bool operator()();
        void check_for_terminal(ATTrie::Position pos);
        void traverse(ATTrie::Position pos);
        bool on_match_helper(Search_StemNode &s1);
        bool isPrefixStemCompatible() const;
        virtual bool onMatch() = 0;

        ~StemEnumerate() {
            if (solution != NULL) {
                delete solution;
            }
        }

        void setSolutionSettings(multiply_params params);
        solution_position *computeFirstSolution();
        bool computeNextSolution(solution_position *current); //compute next posibility
};

#endif
