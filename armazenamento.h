#ifndef ARMAZENAMENTO_H
#define ARMAZENAMENTO_H

void iniciar_spiffs(void);
void salvar_log(const char* data_hora, int temp, int umi, int lum);
void ler_log_serial(void);

#endif