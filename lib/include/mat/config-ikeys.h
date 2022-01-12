/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 *
 * Default ingestion keys for anonymous SDK usage statistics.
 */
#ifndef CONFIG_IKEYS_H
#define CONFIG_IKEYS_H
#if defined(__linux__) || defined(__gnu_linux__)
#define STATS_TOKEN_PROD "4bb4d6f7cafc4e9292f972dca2dcde42-bd019ee8-e59c-4b0f-a02c-84e72157a3ef-7485"
#else
#define STATS_TOKEN_PROD "4bb4d6f7cafc4e9292f972dca2dcde42-bd019ee8-e59c-4b0f-a02c-84e72157a3ef-7485"
#endif
#define STATS_TOKEN_INT  "8130ef8ff472405d89d6f420038927ea-0c0d561e-cca5-4c81-90ed-0aa9ad786a03-7166"
#endif
