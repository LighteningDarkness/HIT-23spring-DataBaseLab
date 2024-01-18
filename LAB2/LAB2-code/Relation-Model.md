航空公司Company(IATA#,公司名cname,类型ctype)
航班Flight(航班号fno#,状态state)
机场Airport(机场名aname#,城市city,类型atype)
航站楼Terminal(机场名aname#,航站楼编号tno#,建成时间ftime)
机库Repository(机场名aname#,编号rno#,面积rs,高度h)
乘客Passenger(身份证号PID#,姓名pname,电话tel)
飞机制造商Producer(注册代码pcode#,国家country,公司名称producername)
执飞飞机Plane(注册号planeid#,机龄age,型号model)
ATC(ATCcode#)

#注意外键
柜台Counter(机场名aname#,航站楼编号tno#,注册代码IATA#,柜台号counterno)
驻扎Stationed(IATA#,机场名aname#)
起飞Takeoff(机场名aname#,航班号fno#,起飞时间ttime)
降落Landon(机场名aname#,航班号fno#,降落时间ltime)
开设Setup(IATA#,航班号fno#)
乘坐ttf(身份证号PID#,航班号fno#)
位于On(ATCcode#,机场名aname#)
管控Control(ATCcode#,航班号fno#,频率f)
执飞Execute(注册号planeid#,航班号fno#,执飞次序exeno)
制造Product(注册代码pcode#,注册号planeid#,产地pcity)
