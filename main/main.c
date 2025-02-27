#include <stdio.h>
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h> // Include for PRIu32

void app_main(void) {
    // 1. Get Chip Information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("Chip Information:\n");
    printf("  Model: %s\n",
           (chip_info.model == CHIP_ESP32)   ? "ESP32" :
           (chip_info.model == CHIP_ESP32S2) ? "ESP32-S2" :
           (chip_info.model == CHIP_ESP32S3) ? "ESP32-S3" :
           (chip_info.model == CHIP_ESP32C3) ? "ESP32-C3" :
           (chip_info.model == CHIP_ESP32H2) ? "ESP32-H2" :
           (chip_info.model == CHIP_ESP32C6) ? "ESP32-C6" : "Unknown");
    printf("  Cores: %d\n", chip_info.cores);
    printf("  Revision: %d\n", chip_info.revision);
    printf("  Features: 0x%" PRIX32 "\n", chip_info.features);
    if (chip_info.features & CHIP_FEATURE_EMB_FLASH) {
        printf("  Embedded Flash: Yes\n");
    } else {
        printf("  Embedded Flash: No\n");
    }
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
        printf("  WiFi: Yes\n");
    } else {
        printf("  WiFi: No\n");
    }

    // 2. Get Flash Size
    // Use esp_flash_default_chip directly (no function call)
    esp_flash_t *flash_chip = esp_flash_default_chip;
    uint32_t flash_size;

    // No need to check for errors *before* getting the size.
    // The esp_flash_get_size function will return an error if needed.
    esp_err_t err = esp_flash_get_size(flash_chip, &flash_size);
    if (err != ESP_OK) {
        printf("Error getting flash size: %s\n", esp_err_to_name(err));
        return;
    }
    printf("Flash Size: %" PRIu32 " bytes (%" PRIu32 " MB)\n", flash_size, flash_size / (1024 * 1024));

    // 3. Get Partition Information
    printf("Partition Information:\n");
    const esp_partition_t *partition = NULL;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

    if (it == NULL) {
        printf("No partitions found!\n");
    } else {
        while (it != NULL) {
            partition = esp_partition_get(it);
            if (partition != NULL) {
                printf("  Label: %s\n", partition->label);
                printf("    Type: 0x%X\n", partition->type);
                printf("    Subtype: 0x%X\n", partition->subtype);
                printf("    Address: 0x%" PRIX32 "\n", partition->address);
                printf("    Size: %" PRIu32 " bytes (%" PRIu32 " KB)\n", partition->size, partition->size / 1024); // Print size in KB
                printf("    Encrypted: %s\n", partition->encrypted ? "Yes" : "No");

                // Print more readable partition type/subtype
                if (partition->type == ESP_PARTITION_TYPE_APP) {
                    printf("    Type: App\n");
                    if (partition->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY) {
                        printf("    Subtype: Factory\n");
                    } else if (partition->subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN &&
                               partition->subtype < ESP_PARTITION_SUBTYPE_APP_OTA_MAX) {
                        printf("    Subtype: OTA_%d\n", partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN);
                    } else {
                        printf("    Subtype: Unknown App Subtype (0x%X)\n", partition->subtype);
                    }

                } else if (partition->type == ESP_PARTITION_TYPE_DATA) {
                    printf("    Type: Data\n");
                    if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_OTA) {
                        printf("    Subtype: OTA Data\n");
                    } else if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_PHY) {
                        printf("    Subtype: PHY Data\n");
                    } else if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_NVS) {
                        printf("    Subtype: NVS\n");
                    } else if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_FAT) {
                         printf("    Subtype: FAT\n");
                    } else if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_SPIFFS) {
                        printf("    Subtype: SPIFFS\n");
                    }
                  else {
                        printf("    Subtype: Unknown Data Subtype (0x%X)\n", partition->subtype);
                    }
                } else {
                    printf("    Type: Unknown Type (0x%X)\n", partition->type);
                }
                printf("    ------------------\n");
            }
            it = esp_partition_next(it);
        }
        esp_partition_iterator_release(it); // Important: Release the iterator
    }


    // 4. Get Running OTA Partition Information (if applicable)
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (running_partition != NULL) {
        printf("Currently Running Partition:\n");
        printf("  Label: %s\n", running_partition->label);
        printf("  Address: 0x%" PRIX32 "\n", running_partition->address);
        printf("  Size: %" PRIu32" bytes\n", running_partition->size);
    }

    printf("Finished.\n");
}