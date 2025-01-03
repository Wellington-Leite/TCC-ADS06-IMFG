# Monitoramento e Controle de Temperatura Utilizando Arduino

## Sobre o Projeto
Este projeto desenvolve um sistema automatizado para monitorar e controlar a temperatura e outros fatores ambientais em um ambiente simulado. Utilizando a plataforma Arduino, sensores e uma interface web, o sistema automatiza o gerenciamento de dispositivos de resfriamento, promovendo eficiência energética e manutenção do ambiente ideal para equipamentos.

## Funcionalidades
- Monitoramento contínuo de temperatura, umidade e gás inflamável.
- Controle automatizado de dispositivos de resfriamento (ex.: ar-condicionado).
- Geração de logs de eventos para análise futura.
- Interface web para monitoramento e alertas em tempo real.

## Tecnologias Utilizadas
- **Hardware:** Arduino Mega 2560, Ethernet Shield W5100, sensores (DHT11, LM35, MQ-2), relés, RTC DS3231.
- **Software:** Programação em C/C++ utilizando o IDE do Arduino.
- **Interface Web:** Desenvolvida com HTML e CSS armazenados em um cartão SD.

## Como Funciona
1. **Sensoriamento:** Captura de dados de temperatura, umidade e presença de gases inflamáveis.
2. **Atuação:** Controle dos dispositivos de resfriamento com base nos dados capturados.
3. **Monitoramento:** Apresentação dos dados em uma página web acessível local ou remotamente.
4. **Registro:** Geração de logs detalhados dos eventos ocorridos no ambiente.

## Resultados
O protótipo foi testado em uma maquete funcional, demonstrando eficiência no controle de temperatura e detecção de anomalias. Resultados indicaram que o sistema pode ser adaptado para aplicações reais com ajustes mínimos.

## Estrutura de Arquivos
- `sketch.ino`: Código principal do Arduino.
- `web/`: Arquivos da interface web.
- `logs/`: Arquivos de registro gerados durante os testes.

## Requisitos de Hardware
- Arduino Mega 2560
- Ethernet Shield W5100
- Sensores: DHT11, LM35, MQ-2
- Relés
- RTC DS3231
- Outros componentes descritos no documento.

## Como Executar
1. Carregue o código no Arduino Mega 2560 utilizando o IDE.
2. Conecte os sensores e atuadores conforme o diagrama de montagem.
3. Configure o servidor web no Ethernet Shield.
4. Acesse a interface web para monitoramento em tempo real.

## Melhorias Futuras
- Substituir o sensor DHT11 por um modelo mais preciso, como o DHT22.
- Implementar um sistema de notificação remota.
- Adaptar para operação em ambientes reais.

## Autor
Wellington Leite de Oliveira  
Desenvolvido como Trabalho de Conclusão de Curso no Instituto Federal de Minas Gerais - Campus Bambuí, 2016.
