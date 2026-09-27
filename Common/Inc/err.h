#ifndef ERR_H
#define ERR_H

#ifdef __cplusplus
extern "C"{
#endif

typedef enum{
    ERR_OK          =0 , /** 成功 */
    ERR_PARAM       =-1, /** 参数错误 */
    ERR_TIMEOUT     =-2, /** 超时错误 */
    ERR_IO          =-3, /** 硬件错误 */
    ERR_CRC         =-4, /** 校验和错误 */
    ERR_NOMEM       =-5, /** 空间不足缓存满 */
    ERR_NOTREADY    =-6, /** 模块未就绪 */
    ERR_NOTFOUND    =-7  /** 未找到目标 */
}err_t;
/**
 * @brief 检查是否码是否成功
 */
#define ERR_IS_OK(x) (ERR_OK == (x))
/**
 * @brief 检查是否码是否失败
 */
#define ERR_IS_FAIL(x) (ERR_OK != (x))


#ifdef __cplusplus
}
#endif

#endif
