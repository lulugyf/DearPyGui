
import pickle
import sys
import struct

import dearpygui.dearpygui as dpg
from dearpygui import _dearpygui
print("file__:", _dearpygui.__file__, dpg.__file__)

lanes_str = '''779 16777217 1.7300000190734863 0.02148520015180111 -0.0002090780035359785 7.609199883518158e-07 0.0 47.599998474121094
782 16777217 -1.7799999713897705 0.020508600398898125 -0.00021103200560901314 7.049699775052432e-07 0.0 42.29999923706055
773 16777219 4.840000152587891 0.02148520015180111 -0.00020712400146294385 7.273500273186073e-07 0.0 47.599998474121094
814 16777219 -3.859999895095825 0.04101720079779625 -0.00013482599752023816 1.9768999948155397e-07 8.300000190734863 44.400001525878906'''

class DWin:
    def __init__(self):
        dpg.create_context()
        self.init_ui()
        self.fvimg: FVImg = None

    def init_ui(self):

        with dpg.window(label="example", on_close=self.w_close, tag='mainwin' ):
            with dpg.group(horizontal=True):
                with dpg.plot(label="", height=850, width=600):
                    dpg.add_plot_legend()
                    xaxis = dpg.add_plot_axis(dpg.mvXAxis, label="", time=False)
                    dpg.set_axis_range(xaxis, -25, 25)
                    with dpg.plot_axis(dpg.mvYAxis, label="") as yaxis:
                        with open("dotcloud.bytes", "rb") as fi:
                            dots = fi.read()  # ffff => (y, z, x, amp)
                        lidar = dpg.add_dot_series(dots, weight=0.2, label="LIDAR")
                        dpg.configure_item(lidar, xoff=0, yoff=-7)
                        
                        dpg.set_axis_range(yaxis, -30, 120)
                        dpg.add_obj_series( [-1.0, 0, 1.0, 2.0], [12, -11, 30, 45],  [97,2,3,177], label="Radar",
                                                id_offsetx=0.2, id_offsety=-0.2, fill=True, show_id=True, weight=2.5, font_size=0.0, shape=2)

                        arr = []
                        for ln in lanes_str.splitlines():
                            f = ln.split()
                            ln_id, ln_type = int(f[0]), int(f[1])
                            (C0, C1, C2, C3, EP, SP) = [float(i) for i in f[2:]]
                            arr.append(struct.pack("iiffffff", ln_id, ln_type, C0, C1, C2, C3, EP, SP))
                        dpg.add_lane_series(b"".join(arr),  weight=2, label="Lanes")


                        dpg.add_span_series(struct.pack('IffIff', 100, -20, -9, 200, -2, 20), label="SPAN")

                        # shape: 1-triangle  2-rectangle  3-circle
                        dpg.add_objstru_series(struct.pack('IffIff', 99, -5, 12, 98, 5, 12), label="Obj", weight=2, fill=True, shape=2)

                        data = b''.join([
                            #                     id   top-left w height  color
                            struct.pack('iffffi', 501, 0, 0,    10, 15, 0xff0000),  # red
                            # struct.pack('iffffi', 502, -15, 30, 10, 15, 0x00ff00),  # green
                            # struct.pack('iffffi', 503, -15, 10, 4, 5, 0x0000ff),  # blue
                        ])
                        dpg.add_rect_series(data, label="rect_serial", fill=True)

                        # 绘制结构体目标, 保存的目标结构文件中的格式
                        with open("obj_f.stru.txt", "r", encoding="utf8") as fi:
                            fmt = fi.readline().strip().encode('utf8')
                        with open("obj_f.stru", "rb") as fi:
                            data = fi.read()
                        self.ob_list = dpg.add_obstacle_series(data, label="obstacle_serial", fill=True, show_angle=True, shape=2, fmt=fmt)
                        dpg.configure_item(self.ob_list, obj_desc="121 What's this?;169 Who am I?\nNext Line")

                        self.draw_dot_lines()

                        dpg.add_light_series([0, 1, 2], label="lights", labels="AEB FCW ELK", size=30, position=2)
                        
                with dpg.group():
                    dpg.add_button(label="test", callback=self.__test)


            with dpg.plot(label="Time Span"):
                id_list = [1, 2, 7, 5, 7]  # 需要排序，否则一些hover光标可能不会显示
                tm = [0.050, 0.200, 0.090, 0.300, 0.010, 0.030, 0.800, 0.900, 80.200, 100.800]
                tm2 = [0.150, 0.400, 0.190, 0.600, 0.110, 0.130, 1.600, 1.800, 160.200, 200.800]
                dpg.add_plot_legend()
                dpg.add_plot_axis(dpg.mvXAxis, label="id", tag="custom_candle_x_axis")
                with dpg.plot_axis(dpg.mvYAxis, label="tm", tag="custom_candle_y_axis"):
                    dpg.add_time_series(id_list, tm, tm2, label="test", tooltip=True)
                    # dpg.add_text("", tag="custom_candle_tooltip")
        dpg.set_primary_window('mainwin', True)

    def draw_dot_lines(self):
        '''点线  IfiI(ff)*size (col-rgba, weight, size, reserve, size*(x,y))'''
        arr = []
        line_header = struct.pack('IfiI', 0xffff0000, 0.12, 5, 5)
        line_dots = b''.join([struct.pack('ff', x, y) for x, y in ((0.0, 0.0), (5,5),(5, 8), (6, 15), (7, 30) )])
        arr.append(line_header + line_dots)
        line_header = struct.pack('IfiI', 0x55ffff00, 5, 5, 9)
        line_dots = b''.join([struct.pack('ff', -x, y) for x, y in ((0.0, 0.0), (5,5),(5, 8), (6, 15), (7, 30) )])
        arr.append(line_header + line_dots)
        
        dpg.add_lane_series(b"".join(arr),  weight=2, line_mode=7, label="Dot-Lines")

    def w_close(self):
        self.closed()

    def closed(self):
        if self.fvimg:
            self.fvimg.close()
            self.fvimg = None
        #print("exited")

    def run(self):
        dpg.create_viewport(title='Custom Title', width=900, height=950)
        dpg.setup_dearpygui()
        dpg.show_viewport()

        # below replaces, start_dearpygui()
        while dpg.is_dearpygui_running():
            # insert here any code you would like to run in the render loop
            # you can manually stop by using stop_dearpygui()
            # print("this will run every frame")
            if self.fvimg is not None:
                tm = int((time.time() - self.start_time) * 1000)
                if self.fvimg.play(tm) != 1:
                    self.w_close() # 播放结束了

            dpg.render_dearpygui_frame()
        self.closed()
        dpg.destroy_context()

    def __test(self, sender, app_data, user_data):
        print(dpg.get_item_configuration(self.ob_list))

def save_example_stru():
    '从生成的目标数据中摘录一段到测试目录下,  以便有一个小的演示数据'
    '''
    struct BaseOB {
    uint32_t id;   // 目标id
    float lat;     // 横向距离,  单位m， 右正
    float lgt;     // 纵向距离， 单位m， 上（前）正
    float width;   // 横向宽度， 单位m
    float length;  // 纵向长度
    int16_t angle;   // 目标运动方向， 单位°， 正前方0,  正右90,  正左-90,  正后180
    uint8_t flag;    // 按位的标志， b0:1-CIPV;  b1: 0-静止 1-运动; b2~b4: 目标类别,  类型对应 OBTYPE; b5~b7: Movement state运动状态
    uint8_t reserve; // 保留
};

struct FIDX{
    unsigned int ms;
    unsigned int sz;
    long long fpos;
};
    '''
    import struct
    import shutil
    import os
    dfile = "D:/record/h97d/20240304175437/misc/obj_f.stru"
    dname = os.path.basename(dfile)
    with open(dfile+".idx", "rb") as fi:
        (ms, sz, fpos) = struct.unpack("IIQ", fi.read(16)) # 读取第一帧的时间和目标数量
    with open(dfile+".txt", "r") as fi:
        fmt = fi.readline().strip()
        stru_sz = int(fi.readline().strip())
    shutil.copy(dfile + ".txt", dname +".txt")
    with open(dname +".idx", "wb") as fo:
        fo.write(struct.pack("IIQ", ms,sz,fpos))
    with open(dfile, "rb") as fi:
        data = fi.read(sz * (stru_sz + 24))
        with open(dname, "wb") as fo:
            fo.write(data)


def main():
    import os
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    w = DWin()
    w.run()

if __name__ == '__main__':
    main()
    # save_example_stru()

