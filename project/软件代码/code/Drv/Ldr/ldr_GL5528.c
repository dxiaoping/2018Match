/*
	�ļ�����ldr_GL5528.c
	���ã������ǿ��ֵ
	ע��GL5528Ϊ���������ͺ�
*/


/**
 *�������������ģ��ADֵ�������ǿ��ֵ��lux��
 *������light_sensor_ad ����ģ��ADֵ
 *����ֵ����ǿ��ֵ��lux��
 *����AD:973 light_sensor_voltege:784 resistance:3.116057 light_sensor_lux:137.238647
 **/
float get_LUX(unsigned short light_sensor_ad)				 
{
	unsigned short light_sensor_voltege;
	float light_sensor_resistance;
	float light_sensor_lux;

	light_sensor_voltege = ((float)light_sensor_ad*3300.0)/4096.0;  //��λ��mV
	light_sensor_resistance = (10.0 * light_sensor_voltege) / (3300.0 - light_sensor_voltege); //��λ��k��
	light_sensor_lux = pow(10, ((log10(15) - log10(light_sensor_resistance) + 0.6) / 0.6));
	return light_sensor_lux;
}
