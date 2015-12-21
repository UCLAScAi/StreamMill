#include "SumUda.h"	/*double quotes tells it to search current directory*/


/*
 *  * Class:     SumUda
 *   * Method:    myiterate
 *    * Signature: (I)I
 *     */
JNIEXPORT jint JNICALL Java_SumUda_myiterate
  (JNIEnv *enc, jobject obj, jint next){
	return 1;
}

/*
 *  * Class:     SumUda
 *   * Method:    terminate
 *    * Signature: (II)I
 *     */
JNIEXPORT jint JNICALL Java_SumUda_terminate
  (JNIEnv *env, jobject obj, jint cur, jint next) {

	return (cur + next);
}

