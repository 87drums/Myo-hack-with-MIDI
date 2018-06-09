# --- coding:utf-8 ---

import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import math #math.floor(x) 小数点以下切り捨て
from statistics import mean, median,variance,stdev

#BD，SD，HHそれぞれに分類したリストを出力
def parse(filename):
    before_time = [0,0,0] #[BD, SD, HH]の前の時間
    output = [[],[],[]] #[[BD],[SD],[HH]]のリスト それぞれ中身は[IOI, 音量]

    Midi_data = np.loadtxt(filename, dtype = "int")

    #print(Midi_data) #MIDI割り当て番号は各配列の2番目に収納
    for row in Midi_data:
        if row[1] == 36:    #BD
            if before_time[0] != 0:
                output[0].append([row[0] - before_time[0], row[1], row[2]])
            before_time[0] = row[0]
        elif row[1] == 38:  #SD
            if before_time[1] != 0:
                output[1].append([row[0] - before_time[1], row[1], row[2]])
            before_time[1] = row[0]
        else:               #HH フットペダル系は排除？ どの操作がどの音高か要調査
            if before_time[2] != 0:
                output[2].append([row[0] - before_time[2], row[1], row[2]])
            before_time[2] = row[0]
    return output

#分散計算
"""
IOIは最小単位を16分音符と仮定し，割った余りの分散でリズムのズレを評価する
音量はどうしよう～～～～～～～～
"""
def calc_var(dat, tem):
    output = [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]] #[[BD IOIvar,音量var],[SD],[HH]]

    #IOIの分散
    min_devide = ((60 * 1000.0) / tem) / 32.0 #最小単位16分音符でのズレ計算の分母
    #print(len(dat)) #使用打楽器の数を返す　今回はBD，SD，HHで3
    for col in range(len(dat)):
        surp = [] #余りlist初期化
        for row in dat[col]:
            surp.append(row[0] % min_devide)
        if len(surp) >= 2:
            output[col][0] = mean(surp)

    #音量の分散
        if len(dat[col]) >= 2:
            #print([row[2] for row in dat[col]])
            #output[col][1] = stdev([row[2] for row in dat[col]])
            output[col][1] = np.std(np.array([row[2] for row in dat[col]]))

    return output

if __name__ == "__main__":
    time = []

    #書き出しファイル名
    OUTPUT_FILENAME = "IOIVel_score.txt"
    time_f = open(OUTPUT_FILENAME, "w")

    #読み込みファイル名指定
    filename_head = "../data/20180501_庵/"
    Midi_filename = filename_head + "MIDI.dat"
    tempo = 140

    MIDI_dat = parse(Midi_filename)
    ans = calc_var(MIDI_dat, tempo)

    print(ans)

    #time_f.write(str(try_time) + "\t")

    time_f.close()
