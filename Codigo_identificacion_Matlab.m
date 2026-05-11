% CAPTURA DE DATOS - TOWER COPTER
clear; clc; close all;

% CONFIGURACIÓN
puerto           = 'COM4';   % Ajusta tu puerto
baudRate         = 9600;
duracionSegundos = 160;      % 7 pasos x 20s + margen

% CONEXIÓN Y CAPTURA
s = serialport(puerto, baudRate);
configureTerminator(s, "LF");
flush(s);

disp('Iniciando captura...');
datos = [];
t_inicio = tic;

while toc(t_inicio) < duracionSegundos
    if s.NumBytesAvailable > 0
        linea = readline(s);
        if contains(linea, 'FIN_SECUENCIA')
            disp('Secuencia completada.');
            break;
        end
        valores = str2double(split(strtrim(linea), ','));
        if numel(valores) == 3 && ~any(isnan(valores))
            datos = [datos; valores'];
        end
    end
end

clear s;

% PROCESAMIENTO
tiempo  = datos(:,1) / 1000;
entrada = datos(:,2);
salida  = datos(:,3);

% GUARDAR
save('datos_tower_copter_final.mat', 'tiempo', 'entrada', 'salida');
disp('Datos guardados en datos_tower_copter_final.mat');

% GRAFICAR DATOS CRUDOS
figure;
subplot(2,1,1);
plot(tiempo, entrada, 'g-', 'LineWidth', 2);
ylabel('PWM (µs)'); xlabel('Tiempo (s)');
title('Señal de entrada'); grid on;

subplot(2,1,2);
plot(tiempo, salida, 'b-', 'LineWidth', 1.5);
ylabel('Altura (cm)'); xlabel('Tiempo (s)');
title('Respuesta del sistema'); grid on;
