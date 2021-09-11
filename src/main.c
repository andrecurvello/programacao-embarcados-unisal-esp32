// 1o - Bibliotecas de referencia para o desenvolvimento do codigo
// Adicionando referencias para chamadas da biblioteca C (printf/scanf)
#include <stdio.h>
// Adicionando referencias para as configuracoes da SDK
#include "sdkconfig.h"
// Adicionando recursos do FreeRTOS para gerenciamento de tarefas (usaremos para delay)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Adicionando recursos para referencia de informacoes do sistema esp32
#include "esp_system.h"
// Adicionando recursos para visualizar informacoes da Flash (SPI)
#include "esp_spi_flash.h"
// Adicionando recursos pra manipular os sinais de entrada e saida (GPIO)
#include "driver/gpio.h"

// 2o - Defines para referencia de dados a serem usados no programa
// Entenda o define como "apelido" - define apelido original
#define LED_PLACA       GPIO_NUM_2
#define BOTAO_1         GPIO_NUM_22
#define LED_CONTROLE    GPIO_NUM_21

// 3o - Variaveis globais (evitem se possivel, mas usem com cuidado)
// uint32_t = unsigned int 32 bits - variavel inteira, sem sinal, com dimensao de 32 bits
uint32_t contador = 0;

// 4o - Prototipos de funcoes presentes no codigo (quando nao usado .h a parte)

// 5o - Implementacao dos metodos e tarefas -----------------------------------

// Callback para tratamento da interrupcao do botao
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    // Verifica se o botao 1 foi a fonte da interrupcao
    // 1 - Como fazer debounce na leitura da interrupcao de gpio?
    if (BOTAO_1 == (uint32_t) arg) //typecast para tipo uint32_t - inteiro usado na definicao dos gpios
    {
        if (gpio_get_level((uint32_t) arg) == 0) 
        {
            contador++;
        }
    }
}


void app_main(void) {
    // Exibe a mensagem no terminal
    printf("Inicializando Esquenta ESP32... \n");

    // Obtemos informacoes do nosso chip:
    // criamos um tipo para armazenar as informacoes
    esp_chip_info_t chip_info;
    // carregamos as informacoes
    esp_chip_info(&chip_info);

    printf("Executando %s com %d CPU Cores - WiFi %s %s \n", 
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    
    printf("Revisao de Silicio: %d \n", chip_info.revision);

    printf("%d MB de Flash %s \n", 
        (spi_flash_get_chip_size() / (1024*1024)),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embarcada" : "externa");

    // Vamos configurar o uso do LED (GPIO 2)
    // "Metodo discreto" de configuracao de GPIOs
    //gpio_reset_pin(LED_PLACA);
    //gpio_set_direction(LED_PLACA, GPIO_MODE_OUTPUT);

    // Metodo de configuracao de pinos usando o gpio_config_t
    // Primeiramente configuramos o botao
    gpio_config_t button_config = {
        .intr_type = GPIO_INTR_NEGEDGE, // interrupcao em borda negativa 1->0
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BOTAO_1),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    // configura o gpio
    gpio_config(&button_config);

    // Vamos configurar "os leds"
    gpio_config_t led_config = {
        .intr_type = GPIO_INTR_DISABLE, //sem interrupcao
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_PLACA) | (1ULL << LED_CONTROLE),
        .pull_down_en = 0
    };
    gpio_config(&led_config);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1); // Estamos configurando uma rotina de tratamento de interrupcao de baixa prioridade
    gpio_isr_handler_add(BOTAO_1, gpio_isr_handler, (void*) BOTAO_1);

    uint8_t estado_led = 0;

    while(1) 
    {
        vTaskDelay( 1000 / portTICK_PERIOD_MS ); // delay de 30ms na API do FreeRTOS, pra debounce de tecla
        printf("Contador: %d\n", contador);
        gpio_set_level(LED_PLACA, estado_led);
        estado_led = !estado_led; // 0->1, 1->0, 1->0...
    }
}