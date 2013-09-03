#include "dname.h"

static char dnames[DNAME_COUNT][DNAME_LENGTH_MAX] = {

    "Abigail", "Brianna", "Christopher", "Daniel", "Emma", "Flora",
    "Grace", "Hannah", "Isabella", "Jacob", "Kenji", "Laura", "Michael",
    "Nicholas", "Olivia", "Paul", "Quentin", "Ryan", "Samantha",
    "Tyler", "Ursula", "Vincent", "William", "Xander", "Yukiko",
    "Zainab",

    "Arlene", "Beth", "Camellia", "Danuta", "Emmaline", "Frank",
    "Gayle", "Hollie", "Isaac", "Jeane", "Kathrine", "Lars",
    "Marianela", "Nichole", "Oscar", "Priscilla", "Quartus", "Rana",
    "Sasha", "Toni", "Ulysses", "Violette", "Walt", "Xerxes",
    "Yehoyada", "Zephaniah",

    "Ava", "Beula", "Carol", "Daren", "Enriqueta", "Francoise",
    "George", "Haggai", "Ira", "John", "Keenan", "Leia",
    "Marie", "Noah", "Obadiah", "Peter", "Quirinius", "Rebecca",
    "Sharyn", "Trisha", "Uzziah", "Velvet", "Wayland", "Xerces",
    "Yessica", "Zoe",

    "Adam", "Bessie", "Carlos", "Dante", "Errol", "Frederic",
    "Giuseppe", "Hai", "Ivan", "Jennifer", "Kiyoshi", "Lelah",
    "Matt", "Nestor", "Owen", "Pamela", "Queenie", "Reynalda",
    "Simon", "Thomas", "Uriel", "Victor", "Weston", "Xibiah",
    "Yoav", "Zinan"

};


const char *
dname(uint16_t id) {
    return dnames[id % DNAME_COUNT];
}

