#!/usr/bin/env python3

# Obtém dados brutos da UART do Pico e os plota conforme recebidos

# Instala dependências:
# python3 -m pip install pyserial matplotlib

# Uso: python3 plotter <port>
# eg. python3 plotter /dev/ttyACM0

# Consulte a API de animação do matplotlib para mais informações: https://matplotlib.org/stable/api/animation_api.html

import serial
import sys
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.lines import Line2D

# disabilita a toolbar
plt.rcParams['toolbar'] = 'None'

class Plotter:
    def __init__(self, ax):
        self.ax = ax
        self.maxt = 250
        self.tdata = [0]
        self.ydata = [3.3/2]
        self.line = Line2D(self.tdata, self.ydata)

        self.ax.add_line(self.line)
        self.ax.set_ylim(0, 3.3)
        self.ax.set_xlim(0, self.maxt)

    def update(self, y):
        lastt = self.tdata[-1]
        if lastt - self.tdata[0] >= self.maxt:  # descarta frames antigos
            self.tdata = self.tdata[1:]
            self.ydata = self.ydata[1:]
            self.ax.set_xlim(self.tdata[0], self.tdata[0] + self.maxt)

        t = lastt + 1
        self.tdata.append(t)
        self.ydata.append(y)
        self.line.set_data(self.tdata, self.ydata)
        return self.line,


def serial_getter():
    # obter novos valores de ADC
    # observe que às vezes a UART perde caracteres, então tentamos no máximo 5 vezes
    # para obter dados adequados
    while True:
        for i in range(5):
            line = ser.readline()
            try:
                line = float(line)
            except ValueError:
                continue
            break
        yield line

if len(sys.argv) < 2:
    raise Exception("Nenhuma porta especificada!")

ser = serial.Serial(sys.argv[1], 115200, timeout=1)

fig, ax = plt.subplots()
plotter = Plotter(ax)

ani = animation.FuncAnimation(fig, plotter.update, serial_getter, interval=1,
                              blit=True, cache_frame_data=False)

ax.set_xlabel("Amostras")
ax.set_ylabel("Tensão (V)")
fig.canvas.manager.set_window_title('Microfone ADC')
fig.tight_layout()
plt.show()
