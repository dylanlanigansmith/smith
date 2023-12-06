#pragma once
#include <stdio.h>
#define LOG(fmt, ...) printf(fmt, __VA_ARGS__); printf("\n")
#define Error(fmt, ...) printf(fmt __VA_ARGS__); printf("\n")