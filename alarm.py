import wave
import struct
import math

# 音频参数
SAMPLE_RATE = 44100
DURATION = 2.0       # 警报时长2秒
FREQ_HIGH = 880     # 高音
FREQ_LOW = 440      # 低音
VOLUME = 32760      # 音量最大值(short范围 -32768~32767)

def gen_alarm_wav():
    wf = wave.open("alarm.wav", "wb")
    wf.setnchannels(1)        # 单声道
    wf.setsampwidth(2)        # 16bit
    wf.setframerate(SAMPLE_RATE)

    frame_count = int(SAMPLE_RATE * DURATION)
    data = []
    switch_cycle = int(SAMPLE_RATE / 8)  # 高低音切换频率 8Hz

    for i in range(frame_count):
        t = i / SAMPLE_RATE
        # 高低音交替蜂鸣
        if (i // switch_cycle) % 2 == 0:
            freq = FREQ_HIGH
        else:
            freq = FREQ_LOW
        sine_val = math.sin(2 * math.pi * freq * t)
        sample = int(sine_val * VOLUME)
        data.append(struct.pack('<h', sample))

    wf.writeframes(b''.join(data))
    wf.close()
    print("已生成 alarm.wav 交替高低音警报音频")

if __name__ == "__main__":
    gen_alarm_wav()