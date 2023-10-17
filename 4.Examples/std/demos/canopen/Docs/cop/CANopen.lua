
------------------------------------------------------------------
-- CANopen CiA 301协议处理
-- 2023-10-10  V1.0  版权: 武汉安富莱电子有限公司
------------------------------------------------------------------

--分窗标志定义
WIN_0 = 0x0001
WIN_1 = 0x0002
WIN_2 = 0x0004
WIN_3 = 0x0008
WIN_4 = 0x0010
WIN_5 = 0x0020
WIN_6 = 0x0040
WIN_7 = 0x0080
WIN_8 =  0x0100
WIN_9 =  0x0200
WIN_10 = 0x0400
WIN_11 = 0x0800
WIN_12 = 0x1000
WIN_13 = 0x2000
WIN_14 = 0x4000
WIN_15 = 0x8000

bin = {}	--全局变量

g_id = 0
g_std_ext = 0
g_data_remote = 0

--CAN 解码函数，返回的字符串将展示到数据窗口
--id 帧ID
--std_ext 0表示标准帧, 1表示扩展帧
--data_remote 0表示数据帧, 1表示远程帧
--datalen 数据长度，字节
--data_bin 二进制数据
--函数返回: ret1解码字符串 , ret2分窗控制字
function can_decoder(id, std_ext, data_remote, datalen, data_bin)
	local ret1 = ""		--返回值1 解码显示结果
	local ret2 = 0  	--返回值2 分窗标志
	local i
	local s0 = ""
	local s1 = ""
	local str_wave = ""
	local idcode
	local ms
	local days

	g_id = id
	g_std_ext = std_ext
	g_data_remote = data_remote


	--#将字符串转换到整数数组存放
	for i = 1,#data_bin,1 do
		bin[i] = tonumber(string.byte(data_bin, i,i))
	end	
	
	g_id = id & 0x0780

	-- 节点状态切换
	if(g_id == 0) then
		s0 = string.format("节点状态切换，")
		if(bin[1] == 0x01) then
			s1 = string.format("0x01启动命令, 让节点进入启动状态")
		elseif (bin[1] == 0x02) then
			s1 = string.format("0x02停止命令, 让节点进入停止状态")
		elseif (bin[1] == 0x80) then
			s1 = string.format("0x80预操作状态, 让节点进预操作状态, PDO关闭, SDO正常")
		elseif (bin[1] == 0x81) then
			s1 = string.format("0x81复位节点应用层")
		elseif (bin[1] == 0x82) then
			s1 = string.format("0x82复位节点通讯, 重新初始化CAN/CANopen")
		else
			s1 = "没找到"
		end

	-- 时间戳报文
	elseif(g_id == 0x0100) then
		g_id = id & 0x007F
		s0 = string.format("时间戳: ")
		ms = (bin[4] << 24) + (bin[3] << 16)+ (bin[2] << 8)  + bin[1]
		days = bin[6] *256 + bin[5]
		s1 = os.date("%Y:%m:%d %H:%M:%S", ms//1000+days*3600*24 + 441763200 - 8*03600)
		s1 = s1..string.format(":%03d", ms%1000)

	-- 节点状态
	elseif(g_id == 0x0700) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x, ", g_id)
		if(bin[1] == 0) then
			s1 = string.format("NMT节点上线报文")
		elseif (bin[1] == 0x04) then
			s1 = string.format("心跳报文, 0x04停止状态")
		elseif (bin[1] == 0x05) then
			s1 = string.format("心跳报文, 0x05操作状态")
		elseif (bin[1] == 0x7f) then
			s1 = string.format("心跳报文, 0x7F预操作状态")
		end
	-- SDO客户端向服务器发送报文，CANopen从机节点是SDO服务器，CANopen主节点是SDO客户端
	elseif(g_id == 0x0600) then
		g_id = id & 0x007F
		s0 = string.format("SDO客户端向服务器节点ID:0x%02x发送报文, ", g_id)
		if(bin[1] == 0x2F) then
			s1 = string.format("SDO命令0x%02x写一个字节数据, 索引0x%04x, 子索引0x%02x, 写对象字典0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])                 
		elseif (bin[1] == 0x2B) then
			s1 = string.format("SDO命令0x%02x写两个字节数据, 索引0x%04x, 子索引0x%02x, 写对象字典0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x27) then
			s1 = string.format("SDO命令0x%02x写三个字节数据, 索引0x%04x, 子索引0x%02x, 写对象字典0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x23) then
			s1 = string.format("SDO命令0x%02x写四个字节数据, 索引0x%04x, 子索引0x%02x, 写对象字典0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x40) then
			s1 = string.format("SDO写命令0x%02x读取数据, 索引0x%04x, 子索引0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4])
		end
	-- SDO服务器向客户端回复报文，CANopen从机节点是SDO服务器，CANopen主节点是SDO客户端
	elseif(g_id == 0x0580) then
		g_id = id & 0x007F
		s0 = string.format("SDO服务器节点ID:0x%02x向客户端回复报文, ", g_id)
		if(bin[1] == 0x4F) then
			s1 = string.format("SDO响应命令0x%02x一个字节数据, 索引0x%04x, 子索引0x%02x, 读对象字典0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5])                 
		elseif (bin[1] == 0x4B) then
			s1 = string.format("SDO响应命令0x%02x两个字节数据, 索引0x%04x, 子索引0x%02x, 读对象字典0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6])
		elseif (bin[1] == 0x47) then
			s1 = string.format("SDO响应命令0x%02x三个字节数据, 索引0x%04x, 子索引0x%02x, 读对象字典0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7])
		elseif (bin[1] == 0x43) then
			s1 = string.format("SDO响应命令0x%02x四个字节数据, 索引0x%04x, 子索引0x%02x, 读对象字典0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x60) then
			s1 = string.format("SDO响应命令0x%02x写成功, 索引0x%04x, 子索引0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4])
		elseif (bin[1] == 0x80) then
			s1 = string.format("SDO响应异常")
		end
	-- TPDO1发送过程数据对象1
	elseif(g_id == 0x0180) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的TPDO1发送过程数据对象1", g_id)
		s1 = " " 
	-- RPDO1接收过程数据对象1
	elseif(g_id == 0x0200) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的RPDO1接收过程数据对象1", g_id)
		s1 = " "
	-- TPDO1发送过程数据对象2
	elseif(g_id == 0x0280) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的TPDO2发送过程数据对象2", g_id)
		s1 = " " 
	-- RPDO1接收过程数据对象2
	elseif(g_id == 0x0300) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的RPDO2接收过程数据对象2", g_id)
		s1 = " "  
	-- TPDO1发送过程数据对象3
	elseif(g_id == 0x0380) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的TPDO3发送过程数据对象3", g_id)
		s1 = " " 
	-- RPDO1接收过程数据对象3
	elseif(g_id == 0x0400) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的RPDO3接收过程数据对象3", g_id)
		s1 = " "  
	-- TPDO1发送过程数据对象4
	elseif(g_id == 0x0480) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的TPDO4发送过程数据对象4", g_id)
		s1 = " " 
	-- RPDO1接收过程数据对象4
	elseif(g_id == 0x0500) then
		g_id = id & 0x007F
		s0 = string.format("节点ID:0x%02x的RPDO4接收过程数据对象4", g_id)
		s1 = " "    
	elseif(g_id == 0x0080) then
		if(id == 0x0080) then
			s0 = "同步报文"
			s1 = " "
		else
			g_id = id & 0x007F
			s0 = string.format("节点ID:0x%02x发紧急报文", g_id)
			idcode = bin[2]*256 +bin[1]

			-----------------------------------------------------
			-----------------------------------------------------
			s1 = string.format("  ##错误代码0x%04x:", idcode)
			if(bin[2] == 0x00) then
				s1 =s1.."错误复位或者没有错误"
			elseif(bin[2] == 0x10) then
				s1 =s1.."一般错误"
			elseif(bin[2] == 0x20) then
				s1 =s1.."电流错误"
			elseif(bin[2] == 0x21) then
				s1 =s1.."电流错误, 设备输入端"
			elseif(bin[2] == 0x22) then
				s1 =s1.."电流错误, 设备内部"
			elseif(bin[2] == 0x23) then
				s1 =s1.."电流错误, 设备输出端"
			elseif(bin[2] == 0x30) then
				s1 =s1.."电压错误"
			elseif(bin[2] == 0x31) then
				s1 =s1.."电压错误, 主供电"
			elseif(bin[2] == 0x32) then
				s1 =s1.."电压错误, 设备内部"
			elseif(bin[2] == 0x33) then
				s1 =s1.."电压错误, 输出"
			elseif(bin[2] == 0x40) then
				s1 =s1.."温度错误"
			elseif(bin[2] == 0x41) then
				s1 =s1.."温度错误，环境"
			elseif(bin[2] == 0x42) then
				s1 =s1.."温度错误，设备"
			elseif(bin[2] == 0x50) then
				s1 =s1.."硬件错误"
			elseif(bin[2] == 0x60) then
				s1 =s1.."软件错误"
			elseif(bin[2] == 0x61) then
				s1 =s1.."软件错误-内部错误"
			elseif(bin[2] == 0x62) then
				s1 =s1.."软件错误-用户错误"
			elseif(bin[2] == 0x63) then
				s1 =s1.."软件错误-数据设置错误"
			elseif(bin[2] == 0x70) then
				s1 =s1.."软件错误-辅助设备错误"
			elseif(bin[2] == 0x80) then
				s1 =s1.."监视错误"
			elseif(bin[2] == 0x81) then
				s1 =s1.."通信错误"
				if(idcode == 0x8110) then
					s1 =s1.."-通信超载"
				elseif(idcode == 0x8120) then
					s1 =s1.."-被动错误"
				elseif(idcode == 0x8130) then
					s1 =s1.."-节点守护错误"
				elseif(idcode == 0x8140) then
					s1 =s1.."-总线关闭恢复"
				end
			elseif(bin[2] == 0x82) then
				s1 =s1.."协议错误"
				if(idcode == 0x8210) then
					s1 =s1.."-PDO没有处理"
				elseif(idcode == 0x8220) then
					s1 =s1.."-长度越界"
				end
			elseif(bin[2] == 0x90) then
				s1 =s1.."外部错误"
			elseif(bin[2] == 0xF0) then
				s1 =s1.."附加功能错误"
			elseif(bin[2] == 0xFF) then
				s1 =s1.."设备特定的错误"
			end	

			-----------------------------------------------------
			-----------------------------------------------------
			s1 = s1..string.format("  ##错误寄存器0x%02x:", bin[3])
			if(bin[3] == 0x01) then
				s1 = s1.."一般错误"
			elseif(bin[3] == 0x02) then
				s1 = s1.."电流错误"
			elseif(bin[3] == 0x04) then
				s1 = s1.."电压错误"
			elseif(bin[3] == 0x08) then
				s1 = s1.."温度错误"
			elseif(bin[3] == 0x10) then
				s1 = s1.."通信错误"
			elseif(bin[3] == 0x20) then
				s1 = s1.."设备特定错误"	
			elseif(bin[3] == 0x40) then
				s1 = s1.."保留, 固定为0"	
			elseif(bin[3] == 0x80) then
				s1 = s1.."厂商特定"				
			end 
			
			-----------------------------------------------------
			-----------------------------------------------------
			s1 = s1..string.format("  ##错误索引0x%02x:", bin[4])
			if(bin[4] == 0x00) then
				s1 = s1.."错误复位或无错误"
			elseif(bin[4] == 0x01) then
				s1 = s1.."达到总线警告限制"
			elseif(bin[4] == 0x02) then
				s1 = s1.."接收到的CAN消息长度错误"
			elseif(bin[4] == 0x03) then
				s1 = s1.."先前接收到的CAN消息还没有处理"
			elseif(bin[4] == 0x04) then
				s1 = s1.."接收PDO数据长度错误"
			elseif(bin[4] == 0x05) then
				s1 = s1.."先前接收到的PDO消息还没有处理"	
			elseif(bin[4] == 0x06) then
				s1 = s1.."CAN接收被动错误"	
			elseif(bin[4] == 0x07) then
				s1 = s1.."CAN发送被动错误"
			elseif(bin[4] == 0x08) then
				s1 = s1.."NMT命令接收错误"
			elseif(bin[4] == 0x09) then
				s1 = s1.."TIME消息超时"
			elseif(bin[4] == 0x12) then
				s1 = s1.."CAN总线发送离线"
			elseif(bin[4] == 0x13) then
				s1 = s1.."CAN接收缓冲溢出"
			elseif(bin[4] == 0x14) then
				s1 = s1.."CAN发送缓冲溢出"
			elseif(bin[4] == 0x15) then
				s1 = s1.."PDO is outside SYNC window"
			elseif(bin[4] == 0x17) then
				s1 = s1.."RPDO消息溢出"					
			elseif(bin[4] == 0x18) then
				s1 = s1.."SYNC消息溢出"				
			elseif(bin[4] == 0x19) then
				s1 = s1.."Unexpected SYNC data length"	
			elseif(bin[4] == 0x1A) then
				s1 = s1.."错误的PDO映射"	
			elseif(bin[4] == 0x1B) then
				s1 = s1.."Heartbeat consumer timeout"
			elseif(bin[4] == 0x1C) then
				s1 = s1.."Heartbeat consumer detected remote node reset"
			elseif(bin[4] == 0x20) then
				s1 = s1.."紧急缓冲满，报文未发送"
			elseif(bin[4] == 0x22) then
				s1 = s1.."单片机刚启动"
			elseif(bin[4] == 0x27) then
				s1 = s1.."自动存储到非易失性存储器失败"
			elseif(bin[4] == 0x28) then
				s1 = s1.."调用函数CO_errorReport参数错误"
			elseif(bin[4] == 0x29) then
				s1 = s1.."定时器任务溢出"
			elseif(bin[4] == 0x2A) then
				s1 = s1.."动态内存申请失败"
			elseif(bin[4] == 0x2B) then
				s1 = s1.."Generic error, test usage"
			elseif(bin[4] == 0x2C) then
				s1 = s1.."软件错误"
			elseif(bin[4] == 0x2D) then
				s1 = s1.."Object dictionary does not match the software"
			elseif(bin[4] == 0x2E) then
				s1 = s1.."Error in calculation of device parameters"
			elseif(bin[4] == 0x2F) then
				s1 = s1.."Error with access to non volatile device memory"
			else
				s1 = s1.."未使用或者未识别"
			end 
		end
	end

	ret1 = s0..s1

	--计算分窗标志, 决定数据包同步显示到哪些子窗口
    ret2 = ret2 | WIN_0	

	return ret1, ret2, str_wave
end



