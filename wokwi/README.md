# Fallout Smart Watch — Wokwi NTP Test

Esta pasta contém uma versão de teste do relógio para o Wokwi.

## Objetivo

Validar primeiro a parte de:

- ESP32
- Wi-Fi simulado do Wokwi (`Wokwi-GUEST`)
- NTP
- Fuso horário de São Paulo (UTC-3)
- Hora em 24 horas
- Data
- Atualização periódica da hora

## Arquivos

- `sketch.ino` — código do relógio/NTP
- `diagram.json` — ESP32 + OLED SSD1306
- `libraries.txt` — bibliotecas usadas pelo display

## Como testar

1. Abra um projeto ESP32 no Wokwi.
2. Copie o conteúdo de `diagram.json` para o diagrama.
3. Use `sketch.ino` como código principal.
4. O Wokwi usa a rede `Wokwi-GUEST`, sem senha.
5. Inicie a simulação.
6. Abra o Serial Monitor em 115200 baud.

O display deve mostrar a hora e a data depois da sincronização NTP.

## Próximo passo

Depois de validar o NTP, vamos trocar o OLED pelo display ST7789 240x240 e aproximar a simulação da Heltec V3 e da interface Fallout do projeto principal.
