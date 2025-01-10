/*
 * ChangeAvailability.cpp
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

#include "Variants.h"

#include "ChangeAvailability.h"
#include "OcppEngine.h"

/*
 * @breif: Instantiate an object ChangeAvailability
 */

ChangeAvailability::ChangeAvailability()
{
}

/*
 * @breif: Method - getOcppOperationType => This method gives the type of Ocpp operation
 */

const char *ChangeAvailability::getOcppOperationType()
{
    return "ChangeAvailability";
}

void ChangeAvailability::processReq(JsonObject payload)
{

    // Required field
    connectorId = payload["connectorId"].as<int>();

    if (connectorId != 1)
    {
        accepted = false;
    }
    else
    {
        accepted = true;
    }
    // Required field
    const char *type = payload["type"] | "Invalid";
    if (!strcmp(type, "Inoperative"))
    {
        // First check the current status.
        if ((getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available || getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Faulted))
        {
            // set the status to unavailable. Status notification will take care of the rest.
            getChargePointStatusService_A()->setUnavailable(true);
            evse_ChargePointStatus = Unavailable;
            accepted = true;
        }
        else
        {
            accepted = false;
        }
    }
    else if (!strcmp(type, "Operative"))
    {
        // change the status if it is unavailable only.
        if ((getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Unavailable))
        {
            // It now becomes available.
            getChargePointStatusService_A()->setUnavailable(false);
             evse_ChargePointStatus = Available;

            accepted = true;
        }
        else
        {
            accepted = false;
        }
    }

    Serial.println("[ChangeAvailability] got the request to change availability");
}

void ChangeAvailability::processConf(JsonObject payload)
{
    /*
     * Change availability update should be processed.
     */
}

DynamicJsonDocument *ChangeAvailability::createReq(JsonObject payload)
{
    /*
     * Ignore Contents of this Req-message, because this is for debug purposes only
     */
}

/*
 * @breif: Added by G. Raja Sumant on the lines of ChangeConfiguration which has
 * 1 field defined as Enum - Accepted/Rejected/Scheduled.
 */
DynamicJsonDocument *ChangeAvailability::createConf()
{
    // As per OCPP 1.6 it is being given similar to ChangeConfiguration
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
