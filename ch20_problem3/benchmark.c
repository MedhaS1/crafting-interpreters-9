#include <stdio.h>
#include <time.h>

#include "common.h"
#include "object.h"
#include "table.h"
#include "vm.h"

#define NUM_KEYS 100000

static ObjString* keys[NUM_KEYS];
static ObjString* missingKeys[NUM_KEYS];

static void makeKeys() {
  char buffer[32];
  for (int i = 0; i < NUM_KEYS; i++) {
    int length = sprintf(buffer, "key%d", i);
    keys[i] = copyString(buffer, length);

    length = sprintf(buffer, "missing%d", i);
    missingKeys[i] = copyString(buffer, length);
  }
}

static void printTime(const char* name, clock_t start) {
  double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
  printf("%-28s %.4f seconds\n", name, seconds);
}

// Inserts a lot of keys so the table has to grow many times.
static void benchInsert() {
  Table table;
  initTable(&table);

  clock_t start = clock();
  for (int i = 0; i < NUM_KEYS; i++) {
    tableSet(&table, keys[i], NUMBER_VAL(i));
  }
  printTime("Insert", start);

  freeTable(&table);
}

// Looks up keys that are all in the table.
static void benchLookupHit() {
  Table table;
  initTable(&table);
  for (int i = 0; i < NUM_KEYS; i++) {
    tableSet(&table, keys[i], NUMBER_VAL(i));
  }

  Value value;
  clock_t start = clock();
  for (int round = 0; round < 10; round++) {
    for (int i = 0; i < NUM_KEYS; i++) {
      tableGet(&table, keys[i], &value);
    }
  }
  printTime("Lookup (found)", start);

  freeTable(&table);
}

// Looks up keys that are not in the table.
static void benchLookupMiss() {
  Table table;
  initTable(&table);
  for (int i = 0; i < NUM_KEYS; i++) {
    tableSet(&table, keys[i], NUMBER_VAL(i));
  }

  Value value;
  clock_t start = clock();
  for (int round = 0; round < 10; round++) {
    for (int i = 0; i < NUM_KEYS; i++) {
      tableGet(&table, missingKeys[i], &value);
    }
  }
  printTime("Lookup (not found)", start);

  freeTable(&table);
}

// Adds and deletes keys over and over so the table fills with tombstones.
static void benchDelete() {
  Table table;
  initTable(&table);

  clock_t start = clock();
  for (int round = 0; round < 10; round++) {
    for (int i = 0; i < NUM_KEYS; i++) {
      tableSet(&table, keys[i], NUMBER_VAL(i));
    }
    for (int i = 0; i < NUM_KEYS; i++) {
      tableDelete(&table, keys[i]);
    }
  }
  printTime("Insert and delete", start);

  freeTable(&table);
}

// Uses a small table a lot of times, like a program with few variables.
static void benchSmallTable() {
  Table table;
  initTable(&table);
  for (int i = 0; i < 8; i++) {
    tableSet(&table, keys[i], NUMBER_VAL(i));
  }

  Value value;
  clock_t start = clock();
  for (int round = 0; round < 125000; round++) {
    for (int i = 0; i < 8; i++) {
      tableGet(&table, keys[i], &value);
    }
  }
  printTime("Small table lookups", start);

  freeTable(&table);
}

int main(int argc, const char* argv[]) {
  initVM();
  makeKeys();

  benchInsert();
  benchLookupHit();
  benchLookupMiss();
  benchDelete();
  benchSmallTable();

  freeVM();
  return 0;
}
