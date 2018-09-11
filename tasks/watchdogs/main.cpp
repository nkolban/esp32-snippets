#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <esp_task_wdt.h>

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

extern "C"
{
    void app_main(void);
}

void highPriorityTask(void *myData)
{
    printf("High priority task started and now looping for 10 seconds.  Our priority is %d.\n", uxTaskPriorityGet(nullptr));
    TickType_t startTicks = xTaskGetTickCount();
    while (xTaskGetTickCount() - startTicks < (10 * 1000 / portTICK_PERIOD_MS))
    {
        // Do nothing but loop
    }
    printf("High priority task ended\n");
    vTaskDelete(nullptr);
}

void hardLoopTask(void *myData)
{
    printf("Hard loop task started ...\n");
    while (1)
    {
        // do nothing but burn CPU
    }
}

void hardLoopTaskNoInterrupts(void *myData)
{
    printf("Hard loop task disabling interrupts started ...\n");
    taskDISABLE_INTERRUPTS();
    while (1)
    {
        // do nothing but burn CPU
    }
}

void myTask(void *myData)
{
    printf("# Running in myTask\n");
    printf("# Registering our new task with the task watchdog.\n");
    esp_task_wdt_add(nullptr);

    printf("# Looping 5 times with a delay of 1 second and not feeding the watchdog.\n");
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
    }

    printf("# Looping 5 times with a delay of 1 second and positively feeding the watchdog.\n");
    esp_task_wdt_reset();
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
        esp_task_wdt_reset();
    }

    printf("# Removing our watchdog registration so we can do something expensive.\n");
    esp_task_wdt_delete(nullptr);

    printf("# Looping 5 times with a delay of 1 second and not feeding the watchdog.\n");
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
    }

    printf("# Re-registering our task with the task watchdog.\n");
    esp_task_wdt_add(nullptr);
    printf("# Looping 5 times with a delay of 1 second and not feeding the watchdog.\n");
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
    }

    printf("# Our current task priority is %d.\n", uxTaskPriorityGet(nullptr));
    printf("# Spwaning a higher priority task\n");
    xTaskCreate(highPriorityTask, // Task code
                "Priority task",  // Name of task
                16 * 1024,        // Stack size
                nullptr,          // Task data
                5,                // Priority
                nullptr           // task handle
    );

    printf("# Looping 5 times with a delay of 1 second and positively feeding the watchdog.\n");
    esp_task_wdt_reset();
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
        esp_task_wdt_reset();
    }

    printf("Spawning a hard-loop function!\n");
    xTaskCreate(hardLoopTaskNoInterrupts, // Task code
                "Hard Loop",  // Name of task
                16 * 1024,    // Stack size
                nullptr,      // Task data
                5,            // Priority
                nullptr       // task handle
    );

    printf("# Looping 5 times with a delay of 1 second and positively feeding the watchdog.\n");
    esp_task_wdt_reset();
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
        esp_task_wdt_reset();
    }


    printf("# Removing our watchdog registration before we end the task.\n");
    esp_task_wdt_delete(nullptr);

    printf("# Ending myTask\n");
    vTaskDelete(nullptr);
} // myTask

void app_main(void)
{
    xTaskHandle handle;
    printf("App starting\n");
    printf("Initializing the task watchdog subsystem with an interval of 2 seconds.\n");
    esp_task_wdt_init(2, false);

    printf("Creatign a new task.\n");
    // Now let us create a new task.
    xTaskCreate(myTask,    // Task code
                "My Task", // Name of task
                16 * 1024, // Stack size
                nullptr,   // Task data
                0,         // Priority
                &handle    // task handle
    );

    //printf("App Ended!\n");
} // app_main
