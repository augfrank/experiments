#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>

#define	DECK_SZ	52 /* A deck of cards is 52 cards */
#define	CARD_SZ	4  /* All the cards are of similar size, say 4 bytes */

/*
 * Define a deck of cards to be a string array of 52 elements and
 * index to the top of the deck
 */
typedef struct
{

    char cards[DECK_SZ][CARD_SZ];
    int top;

} deck_t;

/*
 * This function swaps the contents of two cards.
 */
static void
swapCards(char *s1, char *s2)
{

	char tmp[CARD_SZ];

	/* if any of the input string is NULL, don't bother with the swap */
	if (!s1 || !s2) {
	    return;
	}
	strncpy(tmp, s1, CARD_SZ);
	strncpy(s1, s2, CARD_SZ);
	strncpy(s2, tmp, CARD_SZ);

}

/*
 * This function shuffles a deck of cards. Every element of the deck is
 * swapped with another element that is picked at random. The worst case
 * asymptotic complexity for this function is O(n)
 * This function ensures that every element of the deck is picked at
 * random and therefore is a good shuffle.
 */
void
shuffle(deck_t *xDeck)
{

	int i, j;
	/* reset the deck to be full */
	xDeck->top = DECK_SZ;

	for (i = 0; i < DECK_SZ; i++) {

	    /* picks a new seed for the random number generator every time */
	    srand48(gethrtime());

	    /* pick a random element j between i and DECK_SZ */
	    j = (lrand48() % (DECK_SZ - i)) + i;

	    swapCards(xDeck->cards[i], xDeck->cards[j]);
	}
}

/*
 * This function treats a given card deck as a stack and pops out as
 * many elements as necessary.
 */
void
deal(deck_t *xDeck, int count)
{

	int i;
	for (i = 0; i < count; i++) {
	    if (--xDeck->top < 0) {
		printf("Deck Empty\n");
		return;
	    }
	    printf(" %s ", xDeck->cards[xDeck->top]);
	}
	printf("\n\n");

}

int
main(int argc, char *argv[])
{

	/*
	 * Card Deck
	 * S - Spades
	 * C - Clubs
	 * H - Hearts
	 * D - Diamonds

	 * for Example,
	 * SA is ace of Spades
	 * D3 is 3 of Diamonds
	 * HQ is queen of Hearts
	 */
	deck_t sampleDeck = {
	    {"SA", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9", "S10", "SJ",
	    "SQ", "SK",
	    "CA", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "C10", "CJ",
	    "CQ", "CK",
	    "HA", "H2", "H3", "H4", "H5", "H6", "H7", "H8", "H9", "H10", "HJ",
	    "HQ", "HK",
	    "DA", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "DJ",
	    "DQ", "DK"}, DECK_SZ };

	if (argc < 2 || argc > 2) {
	    printf("Usage: %s <number of cards to deal>\n", argv[0]);
	    return (-1);
	}

	shuffle(&sampleDeck);

	deal(&sampleDeck, atoi(argv[1]));

	return (0);

}
