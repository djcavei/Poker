#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <conio.h>

#define CUORI 'c'
#define QUADRI 'q'
#define FIORI 'f'
#define PICCHE 'p'
#define JACK 11
#define QUEEN 12
#define KING 13
#define ACE 14 || 1
#define TOTAL_CARDS 52
#define TOTAL_SEEDS 4
#define CARDS_PER_SEED 13
#define move_array_size ((2 * playernum) + 5)
#define flop_index (array + (2 * playernum))
#define turn_index (flop_index + 3)
#define river_index (flop_index + 4)
#define HIGH_CARD 100
#define PAIR 200
#define DOUBLE_PAIR 300
#define THREE_OF_A_KIND 400
#define STRAIGHT 500
#define FLUSH 600
#define FULL_HOUSE 700
#define POKER 800
#define ROYAL_FLUSH 1000
#define ALL_IN 2
#define PRE_FLOP state_of_hand == 0
#define RAISE 1
#define CHECK 0
#define FOLD -1

char seeds[4] = {"cqfp"};
int playernum;
int STACK;
int state_of_hand;
int number_of_hands;
int small_blind;
int big_blind;
int dealer_index;
int turn;
int talkers;
int in_hand_player;
int side_pot_array;

typedef struct match {
    int pot;
    int current_target;
    int side_pot;
} match_t;

typedef struct card {
    char seme;
    int valore;
} card_t;

typedef struct player {
    int player;
    card_t *cards;
    int stack;
    double win;
    int in_out;
    char name[20];
    int side_pot;
    int state; /*definisce se sta raisando? boh!*/
    int last_bet;
    int how_many_times;
    int all_in_bet;
} player_t;

match_t *game_create(match_t *g) {
    match_t *x;
    x = (match_t*)malloc(sizeof(match_t));
    x->pot = 0;
    x->side_pot = 0;
    return x;
}

card_t *create_deck(card_t *c) {
    card_t *d;
    int i = 0, j, k;
    d = (card_t*)malloc(TOTAL_CARDS * sizeof (card_t));
    for (j = 0; j < TOTAL_SEEDS; j++) {
        for (k = 1; k <= CARDS_PER_SEED; k++) {
            d[i].seme = seeds[j];
            d[i].valore = k;
            i++;
        }
    }
    return d;
}

player_t *player_cards_initialize(player_t *p) {
    player_t *x;
    int i;
    x = (player_t*)malloc(playernum * sizeof(player_t));
    for (i = 0; i < playernum; i++) {
        x[i].player = i + 1;
        x[i].cards = (card_t*)malloc(2 * sizeof(card_t));
        x[i].stack = STACK;
        x[i].in_out = 1;
        x[i].last_bet = 0;
        x[i].state = 1;
        x[i].how_many_times = 0;
    }
    return x;
}

player_t *elimination(player_t *player) {
    int i, size = playernum;
    for (i = 0; i < playernum; i++) {
        if (player[i].stack <= 0) {
            --size;
        }
    }
    if (size >= playernum) {
        return player;
    }
    else {
        int j;
        player_t *new_player;
        player_t temp_player[size];
        j = 0;
        new_player = (player_t*)malloc(size * sizeof(player_t));
        for (i = 0; i < playernum; i++) {
            if (player[i].stack > 0) {
                new_player[j] = temp_player[j] = player[i];
                ++j;
            }
        }
        free(player);
        playernum = size;
        return new_player;
    }
}

void turn_update() {
    turn = (turn + 1) % playernum;
}

int *array_init(int *arr) {
    int *a;
    a = (int*)malloc(((2 * playernum) + 5) * sizeof(int));
    return a;
}

void hand_initialize(card_t *deck, player_t *p, int *array) {
    int i, j;
    for (i = 0; i < move_array_size; i++) {
        array[i] = rand() % TOTAL_CARDS;
    }
    for (i = 0; i < move_array_size; i++) {
        for (j = 0; j < move_array_size; j++) {
            if (i == j) continue;
            if (array[i] == array[j]) {
                array[i] = rand() % TOTAL_CARDS;
                j = -1;
                continue;
            }
        }
    }
    j = 0;
    for (i = 0; i < playernum; i++) {
        p[i].cards[0] = deck[array[j++]]; /*TROVARE COME ASSEGNARE ED ESCLUDERE*/
        p[i].cards[1] = deck[array[j++]];
    }
    /*da quia fine serve solo per checkprint*/
    /*for (i = 0; i < move_array_size; i++) {
        printf("\n%d", array[i]);
    }*/ /*
    for (i = 0; i < playernum; i++) {
        printf("\n%d, %d", p[i].cards[0].valore, p[i].cards[1].valore);
        printf("\n %c, %c", p[i].cards[0].seme, p[i].cards[1].seme);
    }*/
}

void flop_print(card_t  *dek, const *arr, int len) {
if (len == 0) return;
state_of_hand = 3;
printf("%d%c, ", dek[arr[0]].valore, dek[arr[0]].seme);
flop_print(dek, arr+1, len-1);
}

void turn_print(card_t  *dek, const *arr) {
state_of_hand = 4;
printf("%d%c, ", dek[arr[0]].valore, dek[arr[0]].seme);
}

void river_print(card_t  *dek, const *arr) {
state_of_hand = 5;
printf("%d%c ", dek[arr[0]].valore, dek[arr[0]].seme);
}

void talkers_check(player_t *player) {
    int i;
    talkers = 0;
    for (i = 0; i < playernum; i++) {
        if (player[i].in_out == 1) {
            ++talkers;
        }
    }
}

double flush_check(int *fourteen_card_array, card_t *deck, player_t *player, int index, int flag, int *array_index) {
    int i, j, c_cnt = 0, q_cnt = 0, f_cnt = 0, p_cnt = 0, max_count = 0, count = 0;
    char check = 'x';
    card_t *temp_card;
    temp_card = (card_t*)malloc((2+state_of_hand) * sizeof(card_t));
    for (i = 0; i < 2; i++) {
        temp_card[i] = player[index].cards[i];
    }
    for (j = 0; j < state_of_hand; j++) {
        temp_card[i] = deck[array_index[(2*playernum) + j]];
        i++;
    }
    for (i = 0; i < (2 + state_of_hand); i++) {
        switch (temp_card[i].seme) {
            case 'c': {
                c_cnt++;
                break;
            }
            case 'q': {
                q_cnt++;
                break;
            }
            case 'f': {
                f_cnt++;
                break;
            }
            case 'p': {
                p_cnt++;
                break;
            }
        }
    }
    if (c_cnt >= 5) {
        check = 'c';
    } else if (q_cnt >= 5) {
        check = 'q';
    } else if (f_cnt >=5) {
        check = 'f';
    } else if (p_cnt >= 5) {
        check = 'p';
    }
    if (check != 'x') {
        for (i = 0; i < (2 + state_of_hand); i++) {
            if (temp_card[i].seme == check && temp_card[i].valore > max_count) {
                max_count = temp_card[i].valore;
            }
        }
        if (!flag) {
            free(temp_card);
            printf("FLUSH  ");
            return FLUSH + max_count;
        } else {
            int royal_array[14] = {0};
            for (i = 0; i < 2 + state_of_hand; i++) {
                if (temp_card[i].seme == check) {
                    royal_array[temp_card[i].valore - 1] += 1;
                }
            }
            for (i = 13; i >= 0; i--) {
                if (royal_array[i] == 1) {
                    count++;
                    if (count == 5) {
                        free(temp_card);
                        printf("ROYAL FLUSH  ");
                        return ROYAL_FLUSH + i + 5;
                    }
                } else count = 0;
            }
            printf("FLUSH  ");
            return FLUSH + max_count;
        }
    }
    if (flag) {
        free(temp_card);
        printf("STRAIGHT  ");
        return player[index].win;
    }
    free(temp_card);
    return player[index].win;
}


double point(int *fourteen_card_array, player_t *player, card_t *deck, int *array_index) { /*TODO I KICKER INFERIORI AL PRIMO*/
    int i, count = 0, tris_check = 0, pair_check = 0, double_pair_check = 0;
    double high_card = 0.0, point = 0.0, kicker_2 = 0.0, kicker_3 = 0.0, kicker_4 = 0.0, kicker_5 = 0.0;
    for (i = 0; i < 14; i++) {
        if (fourteen_card_array[i] == 4) {
            return POKER + i + 1;
        }
        if (fourteen_card_array[i] == 3) {
            tris_check = i;
        }
        if (fourteen_card_array[i] == 2) {
            if (!pair_check) {
                pair_check = i;
            } else {
                if (!double_pair_check) {
                    double_pair_check = i;
                } else {
                    pair_check = double_pair_check;
                    double_pair_check = i;
                }
            }
        }
        if (fourteen_card_array[i] >= 1) {
            if (fourteen_card_array[i] == 1) {
                kicker_5 = kicker_4 / 10.00;
                kicker_4 = kicker_3 / 10.00;
                kicker_3 = kicker_2 / 10.00;
                kicker_2 = high_card / 10.00;
                high_card = (double)i/100 + 0.01;
            }
            count++;
            if (count >= 5) {
                point = STRAIGHT; /*SE HAI SCALA L'UNICO PUNTO CHE PUOI AVERE E' FLUSH*/
            }
        } else {
            count = 0;
        }
    }
    if (point) {
        return point + 1 + high_card;
    }
    if (tris_check && (pair_check || double_pair_check)) { printf("FULL HOUSE  ");
        return FULL_HOUSE + tris_check + 1 + high_card;
    }
    else if (tris_check) { printf("THREE OF A KIND  ");
        return THREE_OF_A_KIND + tris_check + 1 + high_card + kicker_2;
    }
    else if (double_pair_check) { printf("DOUBLE PAIR  ");
        return DOUBLE_PAIR + double_pair_check + pair_check + 2 + high_card;
    }
    else if (pair_check) { printf("PAIR  ");
        return PAIR + pair_check + 1 + high_card + kicker_2 + kicker_3;
    }
    else { printf("HIGH CARD  ");
        return HIGH_CARD + (high_card * 100) + 1 + kicker_2 + kicker_3 + kicker_4 + kicker_5;
    }
}

void check_score(player_t *player, card_t *deck, int *array_index) {
    int i, j; /*bitwise?*/
    int fourteen_card_array[14];
    for (i = 0; i < playernum; i++) {
        if (player[i].in_out == 1) { /*MESSO PER NON CONTROLLARE VITTORIA DELL'ELIMINATO*/
            for (j = 0; j < 14; j++) fourteen_card_array[j] = 0;
            for (j = 0; j < state_of_hand; j++) {
                fourteen_card_array[deck[array_index[((2 * playernum) + j)]].valore - 1] += 1;
                if (deck[array_index[((2 * playernum) + j)]].valore == 1) {
                    fourteen_card_array[13] += 1;
                }
            }
            for (j = 0; j < 2; j++) {
                fourteen_card_array[(player[i].cards[j].valore) - 1] += 1;
                if (player[i].cards[j].valore == 1) {
                    fourteen_card_array[13] += 1;
                }
            }
            player[i].win = point(fourteen_card_array, player, deck, array_index);
            if (player[i].win < FULL_HOUSE) {
                if (player[i].win >= STRAIGHT) {
                    player[i].win = flush_check(fourteen_card_array, deck, player, i, 1, array_index);
                } else {
                    player[i].win = flush_check(fourteen_card_array, deck, player, i, 0, array_index);
                }
            }
        }
    }
}

void victory_lies_ahead(player_t *player, match_t *game) {
    int i, i_win = 0;
    double max = 0;
    for (i = 0; i < playernum; i++) {
        if (player[i].in_out == 1) {
            if (player[i].win >= max) {
                max = player[i].win;
                i_win = i;
            }
        }
    }
    player[i_win].stack += game->pot;
}

void blind(player_t *player, match_t *game) {
    player[(dealer_index + 1) % playernum].stack -= small_blind; /*TODO AVRAI PROBLEMI QUANDO ELIMINI UN GIOCATORE PERCHè COME UPDATI IL TURNO?*/
    player[(dealer_index + 1) % playernum].last_bet = small_blind;
    player[(dealer_index + 2) % playernum].stack -= big_blind;
    player[(dealer_index + 2) % playernum].last_bet = big_blind;
    game->pot += (small_blind+big_blind);
    game->current_target = big_blind;
    turn = (dealer_index + 3) % playernum;
}

void clear_all(player_t *player, match_t *game) {
    int i;
    for (i = 0; i < playernum; i++) {
        player[i].win = 0;
        player[i].in_out = 1;
        player[i].last_bet = 0;
        player[i].all_in_bet = 0;
        player[i].side_pot = 0;
        player[i].state = 1;
        game->current_target = 0;
        game->pot = 0;
    }
}

void card_print(player_t *player) {
    int i;
    printf("\n");
    for (i = 0; i < 2; i++) {
        printf("%d%c ", player[turn].cards[i].valore, player[turn].cards[i].seme);
    }
}

void is_there_necessity_for_side_pot(player_t *player, match_t *game, int flag) {
    int i;
/*
    talkers_check(player);
*/
    for (i = 0; i < playernum; i++) {
        if (player[i].stack <= game->current_target - player[i].last_bet && player[i].state == 1 && player[i].in_out == 1) {
            player[i].state = ALL_IN;
            player[i].how_many_times = playernum - 1; /*(playernum - talkers - 1);*/
            if (player[i].stack) {
                player[i].all_in_bet = player[i].stack;
                player[i].side_pot = game->pot - (game->current_target - player[i].stack);
            }
            else {
                player[i].all_in_bet = player[i].last_bet;
                player[i].side_pot = game->pot;
                player[i].state = 3;
            }
        } else if ((player[i].state >= ALL_IN && player[i].how_many_times && flag == 0) || (flag == 3 && player[i].how_many_times && player[i].state == 3)) {
            if (player[i].side_pot && player[(turn + (playernum - 1)) % playernum].in_out) {
                if (player[(turn + (playernum - 1)) % playernum].last_bet >= player[i].all_in_bet) {
                    player[i].side_pot += player[i].all_in_bet;
                } else player[i].side_pot += player[(turn + (playernum - 1)) % playernum].last_bet;
                player[i].how_many_times--;
            }
        }
        printf("\nsidepot %d, how many times %d, allinbet %d", player[i].side_pot, player[i].how_many_times, player[i].all_in_bet);
    }
}

int place_bet(player_t *player, match_t *game) {
    int bet;
    bet = 0;
    if (game->current_target == 0) {
        game->current_target = small_blind;
    }
    printf("\nPLACE YOUR BET: 2. %d, 3. %d, 5. %d, 0. Insert manually (at least %d, max %d (ALL-IN): ", game->current_target * 2, game->current_target * 3, game->current_target * 5, game->current_target * 2, player[turn].stack);
    scanf("\n%d", &bet);
    getchar();
    if (bet == 2 || bet == 3 || bet == 5 || bet == 0) {
        if (bet == 0) {
            printf("Insert bet: ");
            scanf("%d", &bet);
            if (bet >= game->current_target * 2) {
                game->current_target = 1;
            } else return 0;
        }   /*CASO ALL-IN*/
        if (player[turn].stack - ((game->current_target * bet) - player[turn].last_bet) <= 0 && bet >= 2) {
            printf("\n\n\nIVI");
            game->pot += player[turn].stack;
            player[turn].last_bet = player[turn].stack;
            game->current_target = player[turn].last_bet;
            player[turn].stack = 0;
            talkers_check(player);
            --talkers;
            return 1;
        }
        player[turn].stack = player[turn].stack - ((game->current_target * bet) - player[turn].last_bet);
        game->pot = game->pot + ((game->current_target * bet) - player[turn].last_bet);
        player[turn].last_bet = (game->current_target * bet);
        game->current_target = game->current_target * bet;
        talkers_check(player);
        --talkers;
        return 1;
    }
    if (game->current_target == small_blind) {
        game->current_target = 0;
    }
    return 0;
}

int evaluate_option(int option, player_t *player, match_t *game) {
    switch (option) {
        case 1: { /*RAISE BET*/
            return place_bet(player, game);
        }
        case 2: { /*CALL CHECK*/
            player[turn].stack -= (game->current_target - player[turn].last_bet); /* se c'è un current target allora scalerai di quello per la call altirmenti il curr tar sara a 0 e quindi checkerai e basta */
            game->pot += (game->current_target - player[turn].last_bet);
            player[turn].last_bet = game->current_target; /*INVERTITO CON GAMEPOT COME SU BET/RAISE */
            --talkers;
            return 1;
        }
        case 3: {
            player[turn].in_out = 0;
            player[turn].last_bet = 0;
            player[turn].state = 1;
            in_hand_player--;
            --talkers;
            return 1;
        }
        case 9669: {
            game->pot += player[turn].stack;
            player[turn].last_bet = player[turn].stack;
            player[turn].stack = 0;
            player[turn].state = 3;
            --talkers;
            return 1;
        }
        default: return 0;
    }
}

void reset_last_bet(player_t *player) {
    int i;
    for (i = 0; i < playernum; i++){
        player[i].last_bet = 0;
    }
}

void print_check(player_t *player, match_t *game) {
    int i;
    printf("\n");
    for (i = 0; i < playernum; i++) {
        if (player[i].in_out == 1)
        printf("STACK PLAYER %d %s: %d ", player[i].player, player[i].name, player[i].stack);
    }
    printf("POT: %d ", game->pot);
}

int main() {
    setbuf(stdout, 0);
    srand(time(NULL));
    card_t *deck; /*ricorda free*/
    player_t *player; /*ricorda free*/
    match_t *game;
    int option;
    int i;
    int *array; /*TODO ARRAY DELLE CARTE IN GIOCO (2*PLAYERNUM+5)*/
    /*int winner;*/
    playernum = 4;
    STACK = 10000;
    talkers = playernum;
    array = array_init(array); /*TODO RICORDA FREE*/
    player = player_cards_initialize(player);
    deck = create_deck(deck);
    game = game_create(game);
    dealer_index = rand() % playernum;
    state_of_hand = 0;
    option = 0;
    small_blind = 25;
    big_blind = 2*small_blind;
    /*for (i = 0; i < TOTAL_CARDS; i++) {
        printf("\n%d %c", deck[i].valore, deck[i].seme);
    }*/
    for (i = 0; i < playernum; i++) {
        printf("\nInserisci nome player %d: ", i + 1);
        scanf("%s", player[i].name);
        getchar();
    }
    clear_all(player, game);
    while(1) {
        in_hand_player = playernum;
        hand_initialize(deck, player, array);
        blind(player, game); /*TODO PREVEDERE IL CASO DI ALL-IN AL BLIND*/
        while (state_of_hand <= 5) {
            is_there_necessity_for_side_pot(player, game, 0); /*SERVE FARLO SE SKIPPI IL PLAYER RITIRATO?*/
            print_check(player, game);
            if (player[turn].in_out == 0 || player[turn].stack <= 0) {
                turn_update();
                continue;
            }
            printf("\nPOT: %d", game->pot);
            printf("\n\nTURNO GIOCATORE %d %s: ", player[turn].player, player[turn].name);
            printf("\nStack: %d", player[turn].stack);
            printf("\n%d fiches PER CHIAMARE", game->current_target - player[turn].last_bet);
            card_print(player);
            if (in_hand_player == 1) {
                printf("\nVITTORIA GIOCATORE %d %s", player[turn].player, player[turn].name);
                victory_lies_ahead(player, game);
                clear_all(player, game);
                dealer_index = (dealer_index + 1) % playernum;
                state_of_hand = 0;
                break;
            }
            printf("\n1.BET/RAISE 2.CHECK/CALL 3.FOLD");
            printf("\nInsert choice: ");
            scanf("%d", &option);
            getchar();
            if (player[turn].state == 1 && evaluate_option(option, player, game)) {
                turn_update();
            } else if (player[turn].state == ALL_IN) {
                char c = 'x';
                printf("\nAll-in to call: Y/N ");
                while(!kbhit()) {
                    c = getch();
                    if (c == 'y' || c == 'Y') {
                        evaluate_option(9669, player, game);
                    }
                    else {
                        evaluate_option(3, player, game);
                        turn_update();
                    }
                }
            }
            else continue;
            if (!talkers) {
                is_there_necessity_for_side_pot(player, game, 3);
                game->current_target = 0;
                reset_last_bet(player);
                talkers_check(player);
                if (state_of_hand == 0) {
                    printf("\n FLOP: ");
                    flop_print(deck, flop_index, 3);
                } else if (state_of_hand == 3) {
                    printf("\n TURN: ");
                    flop_print(deck, flop_index, 3);
                    turn_print(deck, turn_index);
                }  else if (state_of_hand == 4) {
                    printf("\n RIVER: ");
                    flop_print(deck, flop_index, 3);
                    turn_print(deck, turn_index);
                    river_print(deck, river_index);
                }
                else {
                    check_score(player, deck, array);
                    flop_print(deck, flop_index, 3);
                    turn_print(deck, turn_index);
                    river_print(deck, river_index);
                    for (i = 0; i < playernum; i++) {
                        if (player[i].in_out == 1) {
                            printf("\nPUNTEGGIO %d%c, %d%c  score is: %.5f", player[i].cards[0].valore, /*TODO METTI UN %S PER RETURNARE IL PUNTO IN LETTERE */
                                   player[i].cards[0].seme,
                                   player[i].cards[1].valore, player[i].cards[1].seme, player[i].win);
                        }
                    }
                    victory_lies_ahead(player, game);
                    player = elimination(player);
                    clear_all(player, game);
                    dealer_index = (dealer_index + 1) % playernum;
                    state_of_hand = 0;
                    break;
                }
                turn = (dealer_index + 1) % playernum;
            }
        }
        /*printf("\n FLOP: ");
        flop_print(deck, flop_index, 3);
        talkers = playernum;
        turn = dealer_index + 1;
        turn_print(deck, turn_index);
        river_print(deck, river_index);
        check_score(player, deck, array);
        for (i = 0; i < playernum; i++) {
            printf("\nPUNTEGGIO %d%c, %d%c  score is: %.2f", player[i].cards[0].valore, player[i].cards[0].seme,
                   player[i].cards[1].valore, player[i].cards[1].seme, player[i].win);
        }*/
    }
    return 0;
}