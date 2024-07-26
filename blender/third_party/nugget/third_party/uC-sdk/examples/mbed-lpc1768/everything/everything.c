#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <gpio.h>
#include <ssp.h>
#include <sdcard.h>
#include <BoardConsole.h>
#include <osdebug.h>
#include <stdio.h>
#include <fio.h>
#include <devfs.h>
#include <romfs.h>
#include <semifs.h>
#include <malloc_wrapper.h>
#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/ip_frag.h>
#include <lwip/netif.h>
#include <lwip/init.h>
#include <lwip/stats.h>
#include <lwip/tcp_impl.h>
#include <lwip/timers.h>
#include <netif/etharp.h>
#include <netif/interface.h>
#include <httpserver_raw/httpd.h>
#include <lwip/dhcp.h>

#define LED1_wire make_pin(gpio_port_b, 18)
#define LED2_wire make_pin(gpio_port_b, 20)
#define LED3_wire make_pin(gpio_port_b, 21)
#define LED4_wire make_pin(gpio_port_b, 23)

extern const char romfs[];

static void setupLEDs() {
    gpio_config(LED1_wire, pin_dir_write, pull_up);
    gpio_config(LED2_wire, pin_dir_write, pull_up);
    gpio_config(LED3_wire, pin_dir_write, pull_up);
    gpio_config(LED4_wire, pin_dir_write, pull_up);
}

static void litLED(int led, int value) {
    if ((led > 4) || (led < 1))
        return;

    pin_t ledpin;

    switch (led) {
        case 1: ledpin = LED1_wire; break;
        case 2: ledpin = LED2_wire; break;
        case 3: ledpin = LED3_wire; break;
        case 4: ledpin = LED4_wire; break;
    }

    gpio_set(ledpin, value);
}

static void ledTask(void *p) {
    int n = 0;
    while (1) {
        switch (n++) {
            case 0: litLED(1, 1); litLED(2, 0); litLED(3, 0); litLED(4, 0); break;
            case 1: litLED(1, 0); litLED(2, 1); litLED(3, 0); litLED(4, 0); break;
            case 2: litLED(1, 0); litLED(2, 0); litLED(3, 1); litLED(4, 0); break;
            case 3: litLED(1, 0); litLED(2, 0); litLED(3, 0); litLED(4, 1); break;
            case 4: litLED(1, 0); litLED(2, 0); litLED(3, 1); litLED(4, 0); break;
            case 5: litLED(1, 0); litLED(2, 1); litLED(3, 0); litLED(4, 0); n = 0; break;
        }
        vTaskDelay(200);
    }
}

xSemaphoreHandle handle;

static void simpleTask1(void *p) {
    while (1) {
        xSemaphoreTake(handle, portMAX_DELAY);
        BoardConsolePuts("Task 1");
        xSemaphoreGive(handle);
        vTaskDelay(1234);
    }
}

static void simpleTask2(void *p) {
    while (1) {
        xSemaphoreTake(handle, portMAX_DELAY);
        BoardConsolePuts("Task 2");
        xSemaphoreGive(handle);
        vTaskDelay(1357);
    }
}

#ifdef USE_BAD_TASK
static void badTask(void *x) {
    vTaskDelay(5000);
    char * p = (char *) 0x10000000;
    *p = 42;
}
#endif

static const char msg[] = "Hello world - from fwrite!\r\n";

struct ip_addr board_ip;
static struct netif board_netif;
static xSemaphoreHandle lwip_sem;

#define USE_DHCP

static void net_init() {
    struct ip_addr gw_ip, netmask;
#ifdef USE_DHCP
    IP4_ADDR(&board_ip, 0, 0, 0, 0);
    IP4_ADDR(&gw_ip, 0, 0, 0, 0);
    IP4_ADDR(&netmask, 0, 0, 0, 0);
#else
    inet_aton("192.168.1.2", &board_ip.addr);
    inet_aton("192.168.1.1", &gw_ip.addr);
    inet_aton("255.255.255.0", &netmask.addr);
#endif
    lwip_init();
    vSemaphoreCreateBinary(lwip_sem);

    if (netif_add(&board_netif, &board_ip, &netmask, &gw_ip, NULL, interface_init, ethernet_input) == NULL) {
        fprintf(stderr, "net_init: netif_add(lpc17xx_if_init) failed.\r\n");
        return;
    }
    netif_set_default(&board_netif);

#ifdef USE_DHCP
    printf("Starting DHCP query\n");
    dhcp_start(&board_netif);
#else
    netif_set_up(&board_netif);
#endif
}

// for lwip
uint32_t sys_now() { return xTaskGetTickCount() * portTICK_RATE_MS; }

static void lwip_task(void * p) {
    net_init();
    httpd_init();

    uint32_t currentIP = board_netif.ip_addr.addr;
    while (1) {
        xSemaphoreTake(lwip_sem, 10 / portTICK_RATE_MS);
        interface_check_input(&board_netif);
        sys_check_timeouts();
        if (board_netif.ip_addr.addr != currentIP) {
            currentIP = board_netif.ip_addr.addr;
            printf("got an IP address: %i.%i.%i.%i\n",
                (board_netif.ip_addr.addr >>  0) & 0xff,
                (board_netif.ip_addr.addr >>  8) & 0xff,
                (board_netif.ip_addr.addr >> 16) & 0xff,
                (board_netif.ip_addr.addr >> 24) & 0xff);
        }
    }
}

void ENET_IRQHandler() {
    uint32_t status = LPC_EMAC->IntStatus;
    long woken = pdFALSE;
    LPC_EMAC->IntClear = status;
    xSemaphoreGiveFromISR(lwip_sem, &woken);
    portEND_SWITCHING_ISR(woken);
}

int main() {
    init_malloc_wrapper();
    FILE * f1, * f2;
    char buf[32];
    int c;
    register_devfs();
    register_stdio_devices();
    register_semifs();
    register_romfs("romfs", (uint8_t *) romfs);
    handle = xSemaphoreCreateMutex();
    printf("Hello world - from stdio!\r\n");
    fflush(stdout);
    f1 = fopen("/dev/stdout", "w");
    fprintf(stderr, "f1 = %p\r\n", f1);
    fwrite(msg, 1, sizeof(msg), f1);
    f2 = fopen("/romfs/test.txt", "r");
    if (f2) {
        c = fread(buf, 1, 32, f2);
        printf("read %i bytes.\n", c);
        if (c > 0)
            fwrite(buf, 1, c, f1);
        fclose(f2);
    } else {
        printf("Unable to open /romfs/test.txt.\n");
    }
    FILE * f3 = fopen("/host/TEST.TXT", "r");
    if (f3) {
        c = fread(buf, 1, 32, f3);
        if (c > 0)
            fwrite(buf, 1, c, f1);
        fclose(f3);
    } else {
        printf("Unable to open /host/TEST.TXT.\n");
    }
    fflush(f1);
    fclose(f1);
    setupLEDs();
    litLED(1, 0);
    litLED(2, 0);
    litLED(3, 0);
    litLED(4, 0);
    sdcard_t sdcard;

    pin_t sclk = { .port = gpio_port_a, .pin = 7 };
    pin_t mosi = { .port = gpio_port_a, .pin = 9 };
    pin_t miso = { .port = gpio_port_a, .pin = 8 };
    pin_t cs   = { .port = gpio_port_a, .pin = 6 };

    ssp_port_t ssp = { .ssp = ssp_1, .sclk = sclk, .mosi = mosi, .miso = miso };

    if (sdcard_init(&sdcard, ssp, cs)) {
        printf("Successfully initialized sdcard - reading first sector\n");
        uint8_t data[512];
        if (sdcard_read(&sdcard, data, 0)) {
            int i;
            for (i = 0; i < 512; i++) {
                printf("%02x ", data[i]);
                if ((i & 15) == 15)
                    printf("\n");
            }
            printf("\n");
        } else {
            printf("read error\n");
        }
    }
    printf("Test: %f\n", 12.3456f);
    BoardConsolePuts("Creating simple tasks.");
    xTaskCreate(simpleTask1, (signed char *) "st1", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(simpleTask2, (signed char *) "st2", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY, NULL);
#ifdef USE_BAD_TASK
    xTaskCreate(badTask, (signed char *) "bad", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY, NULL);
#endif
    xTaskCreate(ledTask, (signed char *) "led", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(lwip_task, (signed char *) "lwip", 1024, (void *) NULL, tskIDLE_PRIORITY | portPRIVILEGE_BIT, NULL);
    BoardConsolePuts("Scheduler starting.");
    vTaskStartScheduler();
    BoardConsolePuts("Scheduler exitting.");
    return 0;
}
