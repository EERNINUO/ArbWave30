/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 [EERNINUO]
 *
 * [This file is part of ArbWave30.]
 *
 * ArbWave30 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * ...
 */

#ifndef __SCPI_DEF_H__
#define __SCPI_DEF_H__

#include "scpi/scpi.h"

#define SCPI_TX_BUFFER_SIZE   256
#define SCPI_TX_TIMEOUT_MS    1000
#define SCPI_RX_BUFFER_SIZE   256
#define SCPI_ERROR_QUEUE_SIZE  8

void ArbWave30_SCPI_Init(scpi_t * context);

#endif /* __SCPI_DEF_H__ */