
------------------------------------------------------------------
-- CANopen CiA 301Э�鴦��
-- 2023-10-10  V1.0  ��Ȩ: �人�������������޹�˾
------------------------------------------------------------------

--�ִ���־����
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

bin = {}	--ȫ�ֱ���

g_id = 0
g_std_ext = 0
g_data_remote = 0

--CAN ���뺯�������ص��ַ�����չʾ�����ݴ���
--id ֡ID
--std_ext 0��ʾ��׼֡, 1��ʾ��չ֡
--data_remote 0��ʾ����֡, 1��ʾԶ��֡
--datalen ���ݳ��ȣ��ֽ�
--data_bin ����������
--��������: ret1�����ַ��� , ret2�ִ�������
function can_decoder(id, std_ext, data_remote, datalen, data_bin)
	local ret1 = ""		--����ֵ1 ������ʾ���
	local ret2 = 0  	--����ֵ2 �ִ���־
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


	--#���ַ���ת��������������
	for i = 1,#data_bin,1 do
		bin[i] = tonumber(string.byte(data_bin, i,i))
	end	
	
	g_id = id & 0x0780

	-- �ڵ�״̬�л�
	if(g_id == 0) then
		s0 = string.format("�ڵ�״̬�л���")
		if(bin[1] == 0x01) then
			s1 = string.format("0x01��������, �ýڵ��������״̬")
		elseif (bin[1] == 0x02) then
			s1 = string.format("0x02ֹͣ����, �ýڵ����ֹͣ״̬")
		elseif (bin[1] == 0x80) then
			s1 = string.format("0x80Ԥ����״̬, �ýڵ��Ԥ����״̬, PDO�ر�, SDO����")
		elseif (bin[1] == 0x81) then
			s1 = string.format("0x81��λ�ڵ�Ӧ�ò�")
		elseif (bin[1] == 0x82) then
			s1 = string.format("0x82��λ�ڵ�ͨѶ, ���³�ʼ��CAN/CANopen")
		else
			s1 = "û�ҵ�"
		end

	-- ʱ�������
	elseif(g_id == 0x0100) then
		g_id = id & 0x007F
		s0 = string.format("ʱ���: ")
		ms = (bin[4] << 24) + (bin[3] << 16)+ (bin[2] << 8)  + bin[1]
		days = bin[6] *256 + bin[5]
		s1 = os.date("%Y:%m:%d %H:%M:%S", ms//1000+days*3600*24 + 441763200 - 8*03600)
		s1 = s1..string.format(":%03d", ms%1000)

	-- �ڵ�״̬
	elseif(g_id == 0x0700) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x, ", g_id)
		if(bin[1] == 0) then
			s1 = string.format("NMT�ڵ����߱���")
		elseif (bin[1] == 0x04) then
			s1 = string.format("��������, 0x04ֹͣ״̬")
		elseif (bin[1] == 0x05) then
			s1 = string.format("��������, 0x05����״̬")
		elseif (bin[1] == 0x7f) then
			s1 = string.format("��������, 0x7FԤ����״̬")
		end
	-- SDO�ͻ�������������ͱ��ģ�CANopen�ӻ��ڵ���SDO��������CANopen���ڵ���SDO�ͻ���
	elseif(g_id == 0x0600) then
		g_id = id & 0x007F
		s0 = string.format("SDO�ͻ�����������ڵ�ID:0x%02x���ͱ���, ", g_id)
		if(bin[1] == 0x2F) then
			s1 = string.format("SDO����0x%02xдһ���ֽ�����, ����0x%04x, ������0x%02x, д�����ֵ�0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])                 
		elseif (bin[1] == 0x2B) then
			s1 = string.format("SDO����0x%02xд�����ֽ�����, ����0x%04x, ������0x%02x, д�����ֵ�0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x27) then
			s1 = string.format("SDO����0x%02xд�����ֽ�����, ����0x%04x, ������0x%02x, д�����ֵ�0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x23) then
			s1 = string.format("SDO����0x%02xд�ĸ��ֽ�����, ����0x%04x, ������0x%02x, д�����ֵ�0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x40) then
			s1 = string.format("SDOд����0x%02x��ȡ����, ����0x%04x, ������0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4])
		end
	-- SDO��������ͻ��˻ظ����ģ�CANopen�ӻ��ڵ���SDO��������CANopen���ڵ���SDO�ͻ���
	elseif(g_id == 0x0580) then
		g_id = id & 0x007F
		s0 = string.format("SDO�������ڵ�ID:0x%02x��ͻ��˻ظ�����, ", g_id)
		if(bin[1] == 0x4F) then
			s1 = string.format("SDO��Ӧ����0x%02xһ���ֽ�����, ����0x%04x, ������0x%02x, �������ֵ�0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5])                 
		elseif (bin[1] == 0x4B) then
			s1 = string.format("SDO��Ӧ����0x%02x�����ֽ�����, ����0x%04x, ������0x%02x, �������ֵ�0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6])
		elseif (bin[1] == 0x47) then
			s1 = string.format("SDO��Ӧ����0x%02x�����ֽ�����, ����0x%04x, ������0x%02x, �������ֵ�0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7])
		elseif (bin[1] == 0x43) then
			s1 = string.format("SDO��Ӧ����0x%02x�ĸ��ֽ�����, ����0x%04x, ������0x%02x, �������ֵ�0x%02x, 0x%02x, 0x%02x, 0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4], bin[5], bin[6], bin[7], bin[8])
		elseif (bin[1] == 0x60) then
			s1 = string.format("SDO��Ӧ����0x%02xд�ɹ�, ����0x%04x, ������0x%02x",
								bin[1], bin[3]*256 +bin[2], bin[4])
		elseif (bin[1] == 0x80) then
			s1 = string.format("SDO��Ӧ�쳣")
		end
	-- TPDO1���͹������ݶ���1
	elseif(g_id == 0x0180) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��TPDO1���͹������ݶ���1", g_id)
		s1 = " " 
	-- RPDO1���չ������ݶ���1
	elseif(g_id == 0x0200) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��RPDO1���չ������ݶ���1", g_id)
		s1 = " "
	-- TPDO1���͹������ݶ���2
	elseif(g_id == 0x0280) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��TPDO2���͹������ݶ���2", g_id)
		s1 = " " 
	-- RPDO1���չ������ݶ���2
	elseif(g_id == 0x0300) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��RPDO2���չ������ݶ���2", g_id)
		s1 = " "  
	-- TPDO1���͹������ݶ���3
	elseif(g_id == 0x0380) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��TPDO3���͹������ݶ���3", g_id)
		s1 = " " 
	-- RPDO1���չ������ݶ���3
	elseif(g_id == 0x0400) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��RPDO3���չ������ݶ���3", g_id)
		s1 = " "  
	-- TPDO1���͹������ݶ���4
	elseif(g_id == 0x0480) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��TPDO4���͹������ݶ���4", g_id)
		s1 = " " 
	-- RPDO1���չ������ݶ���4
	elseif(g_id == 0x0500) then
		g_id = id & 0x007F
		s0 = string.format("�ڵ�ID:0x%02x��RPDO4���չ������ݶ���4", g_id)
		s1 = " "    
	elseif(g_id == 0x0080) then
		if(id == 0x0080) then
			s0 = "ͬ������"
			s1 = " "
		else
			g_id = id & 0x007F
			s0 = string.format("�ڵ�ID:0x%02x����������", g_id)
			idcode = bin[2]*256 +bin[1]

			-----------------------------------------------------
			-----------------------------------------------------
			s1 = string.format("  ##�������0x%04x:", idcode)
			if(bin[2] == 0x00) then
				s1 =s1.."����λ����û�д���"
			elseif(bin[2] == 0x10) then
				s1 =s1.."һ�����"
			elseif(bin[2] == 0x20) then
				s1 =s1.."��������"
			elseif(bin[2] == 0x21) then
				s1 =s1.."��������, �豸�����"
			elseif(bin[2] == 0x22) then
				s1 =s1.."��������, �豸�ڲ�"
			elseif(bin[2] == 0x23) then
				s1 =s1.."��������, �豸�����"
			elseif(bin[2] == 0x30) then
				s1 =s1.."��ѹ����"
			elseif(bin[2] == 0x31) then
				s1 =s1.."��ѹ����, ������"
			elseif(bin[2] == 0x32) then
				s1 =s1.."��ѹ����, �豸�ڲ�"
			elseif(bin[2] == 0x33) then
				s1 =s1.."��ѹ����, ���"
			elseif(bin[2] == 0x40) then
				s1 =s1.."�¶ȴ���"
			elseif(bin[2] == 0x41) then
				s1 =s1.."�¶ȴ��󣬻���"
			elseif(bin[2] == 0x42) then
				s1 =s1.."�¶ȴ����豸"
			elseif(bin[2] == 0x50) then
				s1 =s1.."Ӳ������"
			elseif(bin[2] == 0x60) then
				s1 =s1.."�������"
			elseif(bin[2] == 0x61) then
				s1 =s1.."�������-�ڲ�����"
			elseif(bin[2] == 0x62) then
				s1 =s1.."�������-�û�����"
			elseif(bin[2] == 0x63) then
				s1 =s1.."�������-�������ô���"
			elseif(bin[2] == 0x70) then
				s1 =s1.."�������-�����豸����"
			elseif(bin[2] == 0x80) then
				s1 =s1.."���Ӵ���"
			elseif(bin[2] == 0x81) then
				s1 =s1.."ͨ�Ŵ���"
				if(idcode == 0x8110) then
					s1 =s1.."-ͨ�ų���"
				elseif(idcode == 0x8120) then
					s1 =s1.."-��������"
				elseif(idcode == 0x8130) then
					s1 =s1.."-�ڵ��ػ�����"
				elseif(idcode == 0x8140) then
					s1 =s1.."-���߹رջָ�"
				end
			elseif(bin[2] == 0x82) then
				s1 =s1.."Э�����"
				if(idcode == 0x8210) then
					s1 =s1.."-PDOû�д���"
				elseif(idcode == 0x8220) then
					s1 =s1.."-����Խ��"
				end
			elseif(bin[2] == 0x90) then
				s1 =s1.."�ⲿ����"
			elseif(bin[2] == 0xF0) then
				s1 =s1.."���ӹ��ܴ���"
			elseif(bin[2] == 0xFF) then
				s1 =s1.."�豸�ض��Ĵ���"
			end	

			-----------------------------------------------------
			-----------------------------------------------------
			s1 = s1..string.format("  ##����Ĵ���0x%02x:", bin[3])
			if(bin[3] == 0x01) then
				s1 = s1.."һ�����"
			elseif(bin[3] == 0x02) then
				s1 = s1.."��������"
			elseif(bin[3] == 0x04) then
				s1 = s1.."��ѹ����"
			elseif(bin[3] == 0x08) then
				s1 = s1.."�¶ȴ���"
			elseif(bin[3] == 0x10) then
				s1 = s1.."ͨ�Ŵ���"
			elseif(bin[3] == 0x20) then
				s1 = s1.."�豸�ض�����"	
			elseif(bin[3] == 0x40) then
				s1 = s1.."����, �̶�Ϊ0"	
			elseif(bin[3] == 0x80) then
				s1 = s1.."�����ض�"				
			end 
			
			-----------------------------------------------------
			-----------------------------------------------------
			s1 = s1..string.format("  ##��������0x%02x:", bin[4])
			if(bin[4] == 0x00) then
				s1 = s1.."����λ���޴���"
			elseif(bin[4] == 0x01) then
				s1 = s1.."�ﵽ���߾�������"
			elseif(bin[4] == 0x02) then
				s1 = s1.."���յ���CAN��Ϣ���ȴ���"
			elseif(bin[4] == 0x03) then
				s1 = s1.."��ǰ���յ���CAN��Ϣ��û�д���"
			elseif(bin[4] == 0x04) then
				s1 = s1.."����PDO���ݳ��ȴ���"
			elseif(bin[4] == 0x05) then
				s1 = s1.."��ǰ���յ���PDO��Ϣ��û�д���"	
			elseif(bin[4] == 0x06) then
				s1 = s1.."CAN���ձ�������"	
			elseif(bin[4] == 0x07) then
				s1 = s1.."CAN���ͱ�������"
			elseif(bin[4] == 0x08) then
				s1 = s1.."NMT������մ���"
			elseif(bin[4] == 0x09) then
				s1 = s1.."TIME��Ϣ��ʱ"
			elseif(bin[4] == 0x12) then
				s1 = s1.."CAN���߷�������"
			elseif(bin[4] == 0x13) then
				s1 = s1.."CAN���ջ������"
			elseif(bin[4] == 0x14) then
				s1 = s1.."CAN���ͻ������"
			elseif(bin[4] == 0x15) then
				s1 = s1.."PDO is outside SYNC window"
			elseif(bin[4] == 0x17) then
				s1 = s1.."RPDO��Ϣ���"					
			elseif(bin[4] == 0x18) then
				s1 = s1.."SYNC��Ϣ���"				
			elseif(bin[4] == 0x19) then
				s1 = s1.."Unexpected SYNC data length"	
			elseif(bin[4] == 0x1A) then
				s1 = s1.."�����PDOӳ��"	
			elseif(bin[4] == 0x1B) then
				s1 = s1.."Heartbeat consumer timeout"
			elseif(bin[4] == 0x1C) then
				s1 = s1.."Heartbeat consumer detected remote node reset"
			elseif(bin[4] == 0x20) then
				s1 = s1.."����������������δ����"
			elseif(bin[4] == 0x22) then
				s1 = s1.."��Ƭ��������"
			elseif(bin[4] == 0x27) then
				s1 = s1.."�Զ��洢������ʧ�Դ洢��ʧ��"
			elseif(bin[4] == 0x28) then
				s1 = s1.."���ú���CO_errorReport��������"
			elseif(bin[4] == 0x29) then
				s1 = s1.."��ʱ���������"
			elseif(bin[4] == 0x2A) then
				s1 = s1.."��̬�ڴ�����ʧ��"
			elseif(bin[4] == 0x2B) then
				s1 = s1.."Generic error, test usage"
			elseif(bin[4] == 0x2C) then
				s1 = s1.."�������"
			elseif(bin[4] == 0x2D) then
				s1 = s1.."Object dictionary does not match the software"
			elseif(bin[4] == 0x2E) then
				s1 = s1.."Error in calculation of device parameters"
			elseif(bin[4] == 0x2F) then
				s1 = s1.."Error with access to non volatile device memory"
			else
				s1 = s1.."δʹ�û���δʶ��"
			end 
		end
	end

	ret1 = s0..s1

	--����ִ���־, �������ݰ�ͬ����ʾ����Щ�Ӵ���
    ret2 = ret2 | WIN_0	

	return ret1, ret2, str_wave
end



