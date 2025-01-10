/*
 * CLEARCAHCE.CPP
 *
 * Copyright 2022 raja <raja@raja-IdeaPad-Gaming-3-15IMH05>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "ClearCache.h"
#include "OcppEngine.h"

ClearCache::ClearCache()
{
}

const char *ClearCache::getOcppOperationType()
{
    return "ClearCache";
}

DynamicJsonDocument *ClearCache::createReq()
{
    DynamicJsonDocument *doc = new DynamicJsonDocument(0);
    JsonObject payload = doc->to<JsonObject>();
    /*
     * empty payload
     */
    return doc;
}

void ClearCache::processReq(JsonObject payload)
{
    Serial.println(F("[ClearCache]*******Received ClearCache Request|However, nothing to clear*******"));
    accepted = true;
}

void ClearCache::processConf(JsonObject payload)
{
    String status = payload["status"] | "Invalid";

    if (status.equals("Accepted"))
    {
        if (DEBUG_OUT)
            Serial.print("[ClearCache] Request has been accepted!\n");
    }
    else
    {
        Serial.print("[ClearCache] Request has been denied!");
    }
}

DynamicJsonDocument *ClearCache::createConf()
{
    // As per OCPP 1.6J it is being given similar to GetConfiguration
    DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
    JsonObject payload = doc->to<JsonObject>();
    if (accepted)
	{
		payload["status"] = "Accepted";
	}
	else
	{
		payload["status"] = "Rejected";
	}
	accepted = false; // reset the flag
    return doc;
}
