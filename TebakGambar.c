#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UKURAN_PAPAN 4
#define MAKS_PASANGAN 8
#define MAKS_SKOR 100
#define MAKS_PEMAIN 50
#define PANJANG_NAMA 50

typedef struct {
    char simbol;
    int terbuka;
    int cocok;
} Kartu;

typedef struct {
    Kartu papan[UKURAN_PAPAN][UKURAN_PAPAN];
    int skor;
    int gerakan;
    int sisa_pasangan;
    int b1, k1, b2, k2;
} Game;

typedef struct {
    char nama[PANJANG_NAMA];
    int skor;
    int gerakan;
    time_t waktu;
} Pemain;

char simbols[MAKS_PASANGAN] = {'A','B','C','D','E','F','G','H'};
Pemain scoreboard[MAKS_PEMAIN];
int jumlah_pemain = 0;

/* ================= UTIL ================= */

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void tunggu_enter() {
    printf("\nTekan Enter untuk lanjut...");
    clear_input();
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* ================= SCOREBOARD ================= */

void load_scoreboard() {
    FILE *f = fopen("scoreboard.dat", "rb");
    if (f) {
        jumlah_pemain = fread(scoreboard, sizeof(Pemain), MAKS_PEMAIN, f);
        fclose(f);
    }
}

void save_scoreboard() {
    FILE *f = fopen("scoreboard.dat", "wb");
    if (f) {
        fwrite(scoreboard, sizeof(Pemain), jumlah_pemain, f);
        fclose(f);
    }
}

void add_score(const char *nama, int skor, int gerakan) {
    if (jumlah_pemain < MAKS_PEMAIN) {
        strcpy(scoreboard[jumlah_pemain].nama, nama);
        scoreboard[jumlah_pemain].skor = skor;
        scoreboard[jumlah_pemain].gerakan = gerakan;
        scoreboard[jumlah_pemain].waktu = time(NULL);
        jumlah_pemain++;
    }

    for (int i = 0; i < jumlah_pemain - 1; i++) {
        for (int j = i + 1; j < jumlah_pemain; j++) {
            if (scoreboard[j].skor > scoreboard[i].skor ||
               (scoreboard[j].skor == scoreboard[i].skor &&
                scoreboard[j].gerakan < scoreboard[i].gerakan)) {
                Pemain tmp = scoreboard[i];
                scoreboard[i] = scoreboard[j];
                scoreboard[j] = tmp;
            }
        }
    }
    save_scoreboard();
}

void show_scoreboard() {
    clear_screen();
    printf("===== SCOREBOARD =====\n");
    printf("%-4s %-20s %-6s %-8s\n", "No", "Nama", "Skor", "Gerakan");
    printf("--------------------------------\n");
    for (int i = 0; i < jumlah_pemain && i < 10; i++) {
        printf("%-4d %-20s %-6d %-8d\n",
               i + 1,
               scoreboard[i].nama,
               scoreboard[i].skor,
               scoreboard[i].gerakan);
    }
    tunggu_enter();
}

/* ================= GAME ================= */

void init_game(Game *g) {
    g->skor = 0;
    g->gerakan = 0;
    g->sisa_pasangan = MAKS_PASANGAN;

    int idx = 0;
    for (int i = 0; i < UKURAN_PAPAN; i++) {
        for (int j = 0; j < UKURAN_PAPAN; j++) {
            g->papan[i][j].simbol = simbols[idx / 2];
            g->papan[i][j].terbuka = 0;
            g->papan[i][j].cocok = 0;
            idx++;
        }
    }

    for (int i = 0; i < 50; i++) {
        int r1 = rand() % UKURAN_PAPAN;
        int c1 = rand() % UKURAN_PAPAN;
        int r2 = rand() % UKURAN_PAPAN;
        int c2 = rand() % UKURAN_PAPAN;
        Kartu tmp = g->papan[r1][c1];
        g->papan[r1][c1] = g->papan[r2][c2];
        g->papan[r2][c2] = tmp;
    }
}

void show_board(Game *g) {
    printf("    1  2  3  4\n");
    printf("  +--+--+--+--+\n");
    for (int i = 0; i < UKURAN_PAPAN; i++) {
        printf("%d |", i + 1);
        for (int j = 0; j < UKURAN_PAPAN; j++) {
            if (g->papan[i][j].cocok)
                printf(" X|");
            else if (g->papan[i][j].terbuka)
                printf(" %c|", g->papan[i][j].simbol);
            else
                printf(" ?|");
        }
        printf("\n  +--+--+--+--+\n");
    }
}

int input_kartu(const char *msg, int *b, int *k, Game *g) {
    printf("%s", msg);
    if (scanf("%d %d", b, k) != 2) {
        clear_input();
        return 0;
    }
    clear_input();
    (*b)--; (*k)--;

    if (*b < 0 || *b >= UKURAN_PAPAN || *k < 0 || *k >= UKURAN_PAPAN)
        return 0;
    if (g->papan[*b][*k].terbuka || g->papan[*b][*k].cocok)
        return 0;

    return 1;
}

/* ================= MAIN ================= */

int main() {
    srand(time(NULL));
    load_scoreboard();

    char nama[PANJANG_NAMA];
    Game g;

    clear_screen();
    printf("Masukkan nama pemain: ");
    fgets(nama, PANJANG_NAMA, stdin);
    nama[strcspn(nama, "\n")] = 0;
    if (strlen(nama) == 0) strcpy(nama, "Pemain");

    init_game(&g);

    while (g.sisa_pasangan > 0) {
        clear_screen();
        printf("Pemain: %s | Skor: %d | Gerakan: %d\n\n",
               nama, g.skor, g.gerakan);
        show_board(&g);

        if (!input_kartu("\nPilih kartu pertama (baris kolom): ", &g.b1, &g.k1, &g)) {
            printf("Input tidak valid!\n");
            tunggu_enter();
            continue;
        }
        g.papan[g.b1][g.k1].terbuka = 1;

        clear_screen();
        show_board(&g);

        if (!input_kartu("\nPilih kartu kedua (baris kolom): ", &g.b2, &g.k2, &g)) {
            g.papan[g.b1][g.k1].terbuka = 0;
            printf("Input tidak valid!\n");
            tunggu_enter();
            continue;
        }
        g.papan[g.b2][g.k2].terbuka = 1;
        g.gerakan++;

        clear_screen();
        show_board(&g);

        if (g.papan[g.b1][g.k1].simbol == g.papan[g.b2][g.k2].simbol) {
            printf("\nCOCOK! +10 poin\n");
            g.papan[g.b1][g.k1].cocok = 1;
            g.papan[g.b2][g.k2].cocok = 1;
            g.skor += 10;
            g.sisa_pasangan--;
        } else {
            printf("\nTIDAK COCOK!\n");
            tunggu_enter();
            g.papan[g.b1][g.k1].terbuka = 0;
            g.papan[g.b2][g.k2].terbuka = 0;
            continue;
        }
        tunggu_enter();
    }

    clear_screen();
    printf("PERMAINAN SELESAI!\n");
    printf("Skor akhir: %d / %d\n", g.skor, MAKS_SKOR);
    printf("Gerakan  : %d\n", g.gerakan);

    add_score(nama, g.skor, g.gerakan);

    printf("\n1. Lihat scoreboard\n2. Keluar\nPilih: ");
    char p;
    scanf(" %c", &p);
    clear_input();

    if (p == '1')
        show_scoreboard();

    printf("\nTerima kasih sudah bermain!\n");
    return 0;
}