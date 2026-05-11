%% =========================================================
% CAPTURA DE DATOS PID - TOWER COPTER
%% =========================================================

clear;
clc;
close all;

%% =========================
% CONFIGURACIÓN
%% =========================

puerto = 'COM4';     % <-- CAMBIAR
baudrate = 9600;

duracion = 60;       % segundos

%% =========================
% CONEXIÓN SERIAL
%% =========================

s = serialport(puerto, baudrate);

configureTerminator(s,"LF");

flush(s);

disp('Iniciando captura...');

%% =========================
% VARIABLES
%% =========================

datos = [];

t_inicio = tic;

%% =========================
% CAPTURA
%% =========================

while toc(t_inicio) < duracion

    if s.NumBytesAvailable > 0

        linea = readline(s);

        linea = strtrim(linea);

        disp(linea)

        % -------------------------------------------------
        % Extraer datos con sscanf
        % -------------------------------------------------

        valores = sscanf(linea,...
            'SP:%f,H:%f,E:%f,PID:%f,PWM:%f');

        % Verificar datos válidos
        if numel(valores) == 5

            tiempo = toc(t_inicio);

            datos(end+1,:) = [ ...
                tiempo ...
                valores(1) ...
                valores(2) ...
                valores(3) ...
                valores(4) ...
                valores(5)];

        end
    end
end

%% =========================
% CERRAR SERIAL
%% =========================

clear s

disp('Captura finalizada');

%% =========================
% ORGANIZAR VARIABLES
%% =========================

t        = datos(:,1);

setpoint = datos(:,2);

altura   = datos(:,3);

error    = datos(:,4);

pid_out  = datos(:,5);

pwm      = datos(:,6);

%% =========================
% GUARDAR DATOS
%% =========================

save('datos_pid_tower_copter.mat', ...
    't', ...
    'setpoint', ...
    'altura', ...
    'error', ...
    'pid_out', ...
    'pwm');

disp('Datos guardados correctamente');

%% =========================
% GRÁFICAS
%% =========================

figure;

% ---------------------------------------------------------
% Altura vs referencia
% ---------------------------------------------------------
subplot(3,1,1)

plot(t,setpoint,'r--','LineWidth',2)
hold on

plot(t,altura,'b','LineWidth',1.5)

grid on

xlabel('Tiempo [s]')
ylabel('Altura [cm]')

title('Respuesta del sistema')

legend('Setpoint','Altura')

% ---------------------------------------------------------
% Error
% ---------------------------------------------------------
subplot(3,1,2)

plot(t,error,'g','LineWidth',1.5)

grid on

xlabel('Tiempo [s]')
ylabel('Error [cm]')

title('Error del sistema')

% ---------------------------------------------------------
% PWM
% ---------------------------------------------------------
subplot(3,1,3)

plot(t,pwm,'m','LineWidth',1.5)

grid on

xlabel('Tiempo [s]')
ylabel('PWM [us]')

title('Señal de control')

%% =========================
% MÉTRICAS
%% =========================

fprintf('\n========== RESULTADOS ==========\n');

fprintf('Altura máxima: %.2f cm\n',max(altura));

fprintf('Error promedio: %.2f cm\n',mean(abs(error)));

fprintf('PWM mínimo: %.2f\n',min(pwm));

fprintf('PWM máximo: %.2f\n',max(pwm));

fprintf('================================\n');