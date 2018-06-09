# --- coding:utf-8 ---

import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import math #math.floor(x) 小数点以下切り捨て
from statistics import mean, median,variance,stdev

#BD，SD，HHそれぞれに分類したリストを出力
def parse(filename):
    before_time = [0,0,0,0] #[BD, SD, HH, click]の前の時間
    output = [[],[],[]] #[[BD],[SD],[HH]]のリスト それぞれ中身は[unix time, IOI, 音高, 音量]
    click = []

    Midi_data = np.loadtxt(filename, dtype = "int64")

    #print(Midi_data) #MIDI割り当て番号は各配列の2番目に収納
    for row in Midi_data:
        #click
        if row[1] == 22 or row[1] == 24:
            if before_time[3] == 0:
                click.append([row[0], 0, row[1], row[2]])
            else:
                click.append([row[0], row[0] - before_time[0], row[1], row[2]])
            before_time[3] = row[0]

        #BD
        if row[1] == 36:
            if before_time[0] == 0:
                output[0].append([row[0], 0, row[1], row[2]])
            else:
                output[0].append([row[0], row[0] - before_time[0], row[1], row[2]])
            before_time[0] = row[0]

        #SD
        elif row[1] == 38:
            if before_time[1] == 0:
                output[1].append([row[0], 0, row[1], row[2]])
            else:
                output[1].append([row[0], row[0] - before_time[1], row[1], row[2]])
            before_time[1] = row[0]

        #HH フットペダル系(44 04 83)は排除 どの操作がどの音高か要調査 46がどうなってるのか謎なのでひとまず無視
    elif row[1] == 42 or row[1] == 46 or row[1] == 78 or row[1] == 79:
            if before_time[2] == 0:
                output[2].append([row[0], 0, row[1], row[2]])
            else:
                output[2].append([row[0], row[0] - before_time[2], row[1], row[2]])
            before_time[2] = row[0]
    return output, click

#標準偏差計算
"""
打叩タイミングはジャスト拍とのズレで評価（三連はとりあえず考えない）
音量はクラスタ分けし，それぞれで標準偏差計算
"""
#音量のクラス分けのあたりをつけるためのとりあえずプロット
def velPlot(dat, cli, tem):
    row_num, col_num = 2, 2 #パーツ数で自動的に割り当てられるように計算させたい～　必要になったらあとでやる部分
    fig,axes = plt.subplots(nrows=row_num,ncols=col_num,figsize=(10,8)) #プロット用変数　2*2の描画領域確保

    for i in range(len(dat)):
        plot_data = [[],[]] #plot用配列初期化 [[x軸配列],[ｙ軸配列]]
        for row in dat[i]:
            plot_data[0].append(row[3])
            plot_data[1].append(1) #取り合えず横にそろえてプロット，後々クリックとのずれ時間とかをここに入れるかも

        #プロット部
        axes_row, axes_col = i // col_num, i % col_num #動的に描画領域割り当て
        axes[axes_row, axes_col].plot(plot_data[0], plot_data[1], '.')
        axes[axes_row, axes_col].set_title(str(i))
        axes[axes_row, axes_col].set_xlabel('vel')
        axes[axes_row, axes_col].set_ylabel('IOI()')
        axes[axes_row, axes_col].set_xlim(0, 127)
        axes[axes_row, axes_col].grid(True)

    fig.savefig("../data/test.png")
    fig.show()

"""
#IOIと音量の分散をそのまま算出
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
"""

if __name__ == "__main__":
    #書き出しファイル名
    OUTPUT_FILENAME = "IOIVel_score.txt"
    time_f = open(OUTPUT_FILENAME, "w")

    #読み込みファイル名指定
    filename_head = "../data/20180515_test/"
    Midi_filename = filename_head + "test_MIDI.dat"
    tempo = 120

    MIDI_dat, click_list = parse(Midi_filename)
    velPlot(MIDI_dat, click_list, tempo)
    """
    ans = calc_var(MIDI_dat, click_list, tempo)
    print(ans)
    """

    #time_f.write(str(try_time) + "\t")

    time_f.close()
