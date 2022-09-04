#include "pid.h"
#include "vars.h"
#include "uart.h"
#include "gpio.h"
#include <stdio.h>

double saida_medida, sinal_de_controle;
double referencia = 0.0;
double Kp = 0.0; // Ganho Proporcional
double Ki = 0.0; // Ganho Integral
double Kd = 0.0; // Ganho Derivativo
int T = 1.0;     // Período de Amostragem (ms)
unsigned long last_time;
double erro_total, erro_anterior = 0.0;
int sinal_de_controle_MAX = 100.0;
int sinal_de_controle_MIN = -100.0;

void pid_configura_constantes(double Kp_, double Ki_, double Kd_)
{
    Kp = Kp_;
    Ki = Ki_;
    Kd = Kd_;
}

void pid_atualiza_referencia(float referencia_)
{
    referencia = (double)referencia_;
}

double pid_controle(double saida_medida)
{

    double erro = referencia - saida_medida;

    erro_total += erro; // Acumula o erro (Termo Integral)

    if (erro_total >= sinal_de_controle_MAX)
    {
        erro_total = sinal_de_controle_MAX;
    }
    else if (erro_total <= sinal_de_controle_MIN)
    {
        erro_total = sinal_de_controle_MIN;
    }

    double delta_error = erro - erro_anterior; // Diferença entre os erros (Termo Derivativo)

    sinal_de_controle = Kp * erro + (Ki * T) * erro_total + (Kd / T) * delta_error; // PID calcula sinal de controle

    if (sinal_de_controle >= sinal_de_controle_MAX)
    {
        sinal_de_controle = sinal_de_controle_MAX;
    }
    else if (sinal_de_controle <= sinal_de_controle_MIN)
    {
        sinal_de_controle = sinal_de_controle_MIN;
    }

    erro_anterior = erro;

    return sinal_de_controle;
}

void run_pid()
{
    pid_configura_constantes(30.0, 0.2, 400.0);
    unsigned char output1[20], output2[20];
    while (running == 1 && current_time)
    {
        while(key){}
        key=1;
        printf("Running: %d\tCurrent time= %d\n", running, current_time);
        printf("Tentando ler temp interna\n");
        requestData(0x23, 0xC1, uart0_filestream);
        delay(500);
        readOutput(0xC1, uart0_filestream, output1);
        internal_temp = getFloatOutput(output1);
        printf("Tentando ler temp de ref\n");
        sleep(1);
        requestData(0x23, 0xC2, uart0_filestream);
        delay(500);
        readOutput(0xC2, uart0_filestream, output2);
        reference_temp = getFloatOutput(output2);
        printf("Ref tempo: %f\tInternal Temp: %f\tExternal Temp: %f\n", reference_temp, internal_temp, external_temp);
        pid_atualiza_referencia(reference_temp);
        double new_value = pid_controle(internal_temp);
        if (reference_temp > internal_temp)
        {
            setResistance(100);
            setFan(0);
            new_value = 100;
            sendInt(0x16, 0xD1, new_value, uart0_filestream);
        }
        else
        {
            setResistance(0);
            setFan(100);
            new_value = -100;
            sendInt(0x16, 0xD1, new_value, uart0_filestream);
        }
        key=0;
        delay(500);
    }
}