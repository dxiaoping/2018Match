/*
	文件名：ldr_GL5528.c
	作用：计算光强度值
	注：GL5528为光敏电阻型号
*/


/**
 *描述：输入光照模块AD值，输出光强度值（lux）
 *参数：light_sensor_ad 光照模块AD值
 *返回值：光强度值（lux）
 *例：AD:973 light_sensor_voltege:784 resistance:3.116057 light_sensor_lux:137.238647
 **/
float get_LUX(unsigned short light_sensor_ad)				 
{
	unsigned short light_sensor_voltege;
	float light_sensor_resistance;
	float light_sensor_lux;

	light_sensor_voltege = ((float)light_sensor_ad*3300.0)/4096.0;  //单位：mV
	light_sensor_resistance = (10.0 * light_sensor_voltege) / (3300.0 - light_sensor_voltege); //单位：kΩ
	light_sensor_lux = pow(10, ((log10(15) - log10(light_sensor_resistance) + 0.6) / 0.6));
	return light_sensor_lux;
}
