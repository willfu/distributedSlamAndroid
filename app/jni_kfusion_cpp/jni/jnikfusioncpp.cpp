#include <string.h>
#include <jni.h>
#include <dlfcn.h>


#include <kernels.h>
#include <interface.h>
#include <stdint.h>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <time.h>
#include <csignal>

#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>
#include <getopt.h>
#include <unistd.h>

#include <jni.h>
#include <errno.h>




#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>
#include <getopt.h>

#include <thread>
#include <chrono>


#define PREPROCESSING2TRACKING 1
#define TRACKING2INTEGRATION 2
#define TRACKING2RAYCASTING 3
#define INTEGRATION2RAYCASTING 4
#define RAYCASTING2PREPROCESSING 5




#ifdef __ANDROID__
#include <android/log.h>
#ifndef LOGI
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "KFusionCPP", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "KFusionCPP", __VA_ARGS__))
#endif
#else
#define LOGI(...) printf(__VA_ARGS__);
#define LOGW(...) printf(__VA_ARGS__);
#endif


inline double
tock ()
{
  synchroniseDevices ();
#ifdef __APPLE__
  clock_serv_t cclock;
  mach_timespec_t clockData;
  host_get_clock_service (mach_host_self (), SYSTEM_CLOCK, &cclock);
  clock_get_time (cclock, &clockData);
  mach_port_deallocate (mach_task_self (), cclock);
#else
  struct timespec clockData;
  clock_gettime (CLOCK_MONOTONIC, &clockData);
#endif
  return (double) clockData.tv_sec + clockData.tv_nsec / 1000000000.0;
}

uint2 computationSize;
uint2 inputSize;

uint16_t *inputDepth;
uchar3 *raw_rgb;
uchar4 *depthRender;
uchar4 *trackRender;
uchar4 *volumeRender;

DepthReader *reader;
Kfusion *kfusion;
float4 camera;
float3 init_pose;
uint frame = 0;
Configuration *global_config;

double timings[7];

#include <mygltest.h>

extern "C"
{
  JNIEXPORT jboolean JNICALL
    Java_project_pamela_slambench_jni_KFusion_kfusioncppinit (JNIEnv * env,
							      jobject thiz,
							      jobjectArray
							      filename);
  JNIEXPORT jstring JNICALL
    Java_project_pamela_slambench_jni_KFusion_kfusioncppprocess (JNIEnv *
								 env);
  JNIEXPORT jboolean JNICALL
    Java_project_pamela_slambench_jni_KFusion_kfusioncpprelease (JNIEnv *
								 env);

  JNIEXPORT void JNICALL
    Java_project_pamela_slambench_jni_KFusion_kfusioncppglinit (JNIEnv *);
  JNIEXPORT void JNICALL
    Java_project_pamela_slambench_jni_KFusion_kfusioncppglresize (JNIEnv *,
								  jint width,
								  jint
								  height);
  JNIEXPORT void JNICALL
    Java_project_pamela_slambench_jni_KFusion_kfusioncppglrender (JNIEnv *);

          JNIEXPORT jstring JNICALL
            Java_project_pamela_slambench_jni_KFusion_kfusioncppinfo (JNIEnv *
        								 env);

}




JNIEXPORT jstring JNICALL
Java_project_pamela_slambench_jni_KFusion_kfusioncppinfo (JNIEnv * env)
{

      std::ostringstream o;
      o << "Start infos" << std::endl;
      return  (env)->NewStringUTF(o.str().c_str());

}

JNIEXPORT void JNICALL
Java_project_pamela_slambench_jni_KFusion_kfusioncppglrender (JNIEnv *)
{
  Draw ();

}


JNIEXPORT void JNICALL
Java_project_pamela_slambench_jni_KFusion_kfusioncppglinit (JNIEnv *)
{

  Init ();

}

JNIEXPORT void JNICALL
Java_project_pamela_slambench_jni_KFusion_kfusioncppglresize (JNIEnv *,
							      jint width,
							      jint height)
{

  Resize ();

}


JNIEXPORT jboolean JNICALL
Java_project_pamela_slambench_jni_KFusion_kfusioncpprelease (JNIEnv * env)
{

  LOGI ("Start cpp mem release");
  if (raw_rgb)
    free (raw_rgb);
    raw_rgb = NULL;
  if (inputDepth)
    free (inputDepth);
  inputDepth = NULL;
  if (depthRender)
    free (depthRender);
  depthRender = NULL;
  if (trackRender)
    free (trackRender);
  trackRender = NULL;
  if (volumeRender)
    free (volumeRender);
  volumeRender = NULL;
  if (reader)
    delete (reader);
    reader = NULL;
  if (kfusion)
    delete (kfusion);
    kfusion = NULL;
  if (global_config)
    free (global_config);
    global_config = NULL;

  LOGI ("Finish cpp mem release");
  return true;

}

JNIEXPORT jstring JNICALL
Java_project_pamela_slambench_jni_KFusion_kfusioncppprocess (JNIEnv * env)
{

  std::ostringstream o;
  Configuration & config = *global_config;
  timings[0] = tock ();

  if (reader->readNextDepthFrame (raw_rgb, inputDepth))




// preprocessing
 /*
    {


      Matrix4 pose = kfusion->getPose ();

      float xt = pose.data[0].w - init_pose.x;
      float yt = pose.data[1].w - init_pose.y;
      float zt = pose.data[2].w - init_pose.z;

      timings[1] = tock ();

      LOGI("frame %d", frame);
      LOGI ("Before Preprocessing\n");

      kfusion->preprocessing (inputDepth, inputSize, frame);

      timings[2] = tock ();

      LOGI ("Before Tracking\n");
      bool tracked = true; //kfusion->tracking (camera, config.icp_threshold,
					//config.tracking_rate, frame);


      timings[3] = tock ();
      LOGI ("Before Integration\n");
      bool integrated = true;
      //kfusion->integration (camera, config.integration_rate,
	//				      config.mu, frame);


      timings[4] = tock ();

      LOGI("Before Raycast");
      bool raycast = true;//kfusion->raycasting (camera, config.mu, frame);


      timings[5] = tock ();

      //kfusion->renderDepth (depthRender, computationSize);

      //kfusion->renderTrack (trackRender, computationSize);

      //kfusion->renderVolume (volumeRender, computationSize, frame,
			     //config.rendering_rate, camera, 0.75 * config.mu);


      timings[6] = tock ();


      __android_log_print (ANDROID_LOG_INFO, "kfusion", "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%d", frame, timings[1] - timings[0], timings[2] - timings[1],	//  preprocessing
			   timings[3] - timings[2],	//  tracking
			   timings[4] - timings[3],	//  integration
			   timings[5] - timings[4],	//  raycasting
			   timings[6] - timings[5],	//  rendering
			   timings[5] - timings[1],	//  computation
			   timings[6] - timings[0],	//  total
			   xt, yt, zt,	//  X,Y,Z
			   tracked, integrated);

      o <<  frame << "\t"
        << timings[1] - timings[0] << "\t"          << timings[2] - timings[1] << "\t"          <<	//  preprocessing
			   timings[3] - timings[2] << "\t"          <<	//  tracking
			   timings[4] - timings[3] << "\t"          <<	//  integration
			   timings[5] - timings[4] << "\t"          <<	//  raycasting
			   timings[6] - timings[5] << "\t"          <<	//  rendering
			   timings[5] - timings[1] << "\t"          <<	//  computation
			   timings[6] - timings[0] << "\t"          <<	//  total
			   xt << "\t"          << yt << "\t"          << zt << "\t"          <<	//  X << "\t"          <<Y << "\t"          <<Z
			   tracked << "\t"          << integrated;

      frame++;

      timings[0] = tock ();

      return  (env)->NewStringUTF(o.str().c_str());

    }*/

     // trakcing

     /*
      {
         Matrix4 pose = kfusion->getPose ();

         float xt = pose.data[0].w - init_pose.x;
         float yt = pose.data[1].w - init_pose.y;
         float zt = pose.data[2].w - init_pose.z;

         timings[1] = tock ();

         LOGI("frame %d", frame);
         LOGI ("Before Preprocessing\n");

         //kfusion->preprocessing (inputDepth, inputSize, frame);

         timings[2] = tock ();

         LOGI ("Before Tracking\n");
         bool tracked = kfusion->tracking (camera, config.icp_threshold,
   					config.tracking_rate, frame);



         timings[3] = tock ();
         LOGI ("Before Integration\n");
         bool integrated = true;
         //kfusion->integration (camera, config.integration_rate,
   		//			      config.mu, frame);


         timings[4] = tock ();

         LOGI("Before Raycast");
         bool raycast = true;
         //kfusion->raycasting (camera, config.mu, frame);


         timings[5] = tock ();

         //kfusion->renderDepth (depthRender, computationSize);

         //kfusion->renderTrack (trackRender, computationSize);

         //kfusion->renderVolume (volumeRender, computationSize, frame,
   		//	     config.rendering_rate, camera, 0.75 * config.mu);


         timings[6] = tock ();


         __android_log_print (ANDROID_LOG_INFO, "kfusion", "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%d", frame, timings[1] - timings[0], timings[2] - timings[1],	//  preprocessing
   			   timings[3] - timings[2],	//  tracking
   			   timings[4] - timings[3],	//  integration
   			   timings[5] - timings[4],	//  raycasting
   			   timings[6] - timings[5],	//  rendering
   			   timings[5] - timings[1],	//  computation
   			   timings[6] - timings[0],	//  total
   			   xt, yt, zt,	//  X,Y,Z
   			   tracked, integrated);

         o <<  frame << "\t"
           << timings[1] - timings[0] << "\t"          << timings[2] - timings[1] << "\t"          <<	//  preprocessing
   			   timings[3] - timings[2] << "\t"          <<	//  tracking
   			   timings[4] - timings[3] << "\t"          <<	//  integration
   			   timings[5] - timings[4] << "\t"          <<	//  raycasting
   			   timings[6] - timings[5] << "\t"          <<	//  rendering
   			   timings[5] - timings[1] << "\t"          <<	//  computation
   			   timings[6] - timings[0] << "\t"          <<	//  total
   			   xt << "\t"          << yt << "\t"          << zt << "\t"          <<	//  X << "\t"          <<Y << "\t"          <<Z
   			   tracked << "\t"          << integrated;

         frame++;

         timings[0] = tock ();

         return  (env)->NewStringUTF(o.str().c_str());

       }

*/




     // integration
     /*
      {


         Matrix4 pose = kfusion->getPose ();

         float xt = pose.data[0].w - init_pose.x;
         float yt = pose.data[1].w - init_pose.y;
         float zt = pose.data[2].w - init_pose.z;

         timings[1] = tock ();

         LOGI("frame %d", frame);
         LOGI ("Before Preprocessing\n");

         //kfusion->preprocessing (inputDepth, inputSize, frame);

         timings[2] = tock ();

         LOGI ("Before Tracking\n");
         bool tracked = true; //kfusion->tracking (camera, config.icp_threshold,
   					//config.tracking_rate, frame);



         timings[3] = tock ();
         LOGI ("Before Integration\n");
         bool integrated = true;
         kfusion->integration (camera, config.integration_rate,
   					      config.mu, frame);


         timings[4] = tock ();

         LOGI("Before Raycast");
         bool raycast = true;
         //kfusion->raycasting (camera, config.mu, frame);


         timings[5] = tock ();

         //kfusion->renderDepth (depthRender, computationSize);

         //kfusion->renderTrack (trackRender, computationSize);

         //kfusion->renderVolume (volumeRender, computationSize, frame,
   		//	     config.rendering_rate, camera, 0.75 * config.mu);


         timings[6] = tock ();


         __android_log_print (ANDROID_LOG_INFO, "kfusion", "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%d", frame, timings[1] - timings[0], timings[2] - timings[1],	//  preprocessing
   			   timings[3] - timings[2],	//  tracking
   			   timings[4] - timings[3],	//  integration
   			   timings[5] - timings[4],	//  raycasting
   			   timings[6] - timings[5],	//  rendering
   			   timings[5] - timings[1],	//  computation
   			   timings[6] - timings[0],	//  total
   			   xt, yt, zt,	//  X,Y,Z
   			   tracked, integrated);

         o <<  frame << "\t"
           << timings[1] - timings[0] << "\t"          << timings[2] - timings[1] << "\t"          <<	//  preprocessing
   			   timings[3] - timings[2] << "\t"          <<	//  tracking
   			   timings[4] - timings[3] << "\t"          <<	//  integration
   			   timings[5] - timings[4] << "\t"          <<	//  raycasting
   			   timings[6] - timings[5] << "\t"          <<	//  rendering
   			   timings[5] - timings[1] << "\t"          <<	//  computation
   			   timings[6] - timings[0] << "\t"          <<	//  total
   			   xt << "\t"          << yt << "\t"          << zt << "\t"          <<	//  X << "\t"          <<Y << "\t"          <<Z
   			   tracked << "\t"          << integrated;

         frame++;

         timings[0] = tock ();

         return  (env)->NewStringUTF(o.str().c_str());

       }



*/




//raycasting

      {


         Matrix4 pose = kfusion->getPose ();

         float xt = pose.data[0].w - init_pose.x;
         float yt = pose.data[1].w - init_pose.y;
         float zt = pose.data[2].w - init_pose.z;

         timings[1] = tock ();

         LOGI("frame %d", frame);
         LOGI ("Before Preprocessing\n");

         //kfusion->preprocessing (inputDepth, inputSize, frame);

         timings[2] = tock ();

         LOGI ("Before Tracking\n");
         bool tracked = true; //kfusion->tracking (camera, config.icp_threshold,
   					//config.tracking_rate, frame);



         timings[3] = tock ();
         LOGI ("Before Integration\n");
         bool integrated = true;
         //kfusion->integration (camera, config.integration_rate,
   		//			      config.mu, frame);


         timings[4] = tock ();

         LOGI("Before Raycast");
         bool raycast = kfusion->raycasting (camera, config.mu, frame);


         timings[5] = tock ();

         kfusion->renderDepth (depthRender, computationSize);

         kfusion->renderTrack (trackRender, computationSize);

         kfusion->renderVolume (volumeRender, computationSize, frame,
   			     config.rendering_rate, camera, 0.75 * config.mu);


         timings[6] = tock ();


         __android_log_print (ANDROID_LOG_INFO, "kfusion", "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%d", frame, timings[1] - timings[0], timings[2] - timings[1],	//  preprocessing
   			   timings[3] - timings[2],	//  tracking
   			   timings[4] - timings[3],	//  integration
   			   timings[5] - timings[4],	//  raycasting
   			   timings[6] - timings[5],	//  rendering
   			   timings[5] - timings[1],	//  computation
   			   timings[6] - timings[0],	//  total
   			   xt, yt, zt,	//  X,Y,Z
   			   tracked, integrated);

         o <<  frame << "\t"
           << timings[1] - timings[0] << "\t"          << timings[2] - timings[1] << "\t"          <<	//  preprocessing
   			   timings[3] - timings[2] << "\t"          <<	//  tracking
   			   timings[4] - timings[3] << "\t"          <<	//  integration
   			   timings[5] - timings[4] << "\t"          <<	//  raycasting
   			   timings[6] - timings[5] << "\t"          <<	//  rendering
   			   timings[5] - timings[1] << "\t"          <<	//  computation
   			   timings[6] - timings[0] << "\t"          <<	//  total
   			   xt << "\t"          << yt << "\t"          << zt << "\t"          <<	//  X << "\t"          <<Y << "\t"          <<Z
   			   tracked << "\t"          << integrated;

         frame++;

         timings[0] = tock ();

         return  (env)->NewStringUTF(o.str().c_str());

       }








  else
    {
      LOGI ("No more frame\n");
      return  (env)->NewStringUTF(o.str().c_str());

    }
}

std::vector < char *>
stringArrayJ2C (JNIEnv * env, jobjectArray array)
{
  std::vector < char *>vec;
  jsize stringCount = (*env).GetArrayLength (array);

  for (int i = 0; i < stringCount; i++)
    {
      jstring string = (jstring) (*env).GetObjectArrayElement (array, i);
      vec.push_back ((char *) (*env).GetStringUTFChars (string, NULL));
    }
  return vec;
}


jboolean
Java_project_pamela_slambench_jni_KFusion_kfusioncppinit (JNIEnv * env,
							  jobject thiz,
							  jobjectArray
							  stringArray)
{
  LOGI ("Start android_main");

  std::vector < char *>vec = stringArrayJ2C (env, stringArray);
  for (unsigned int i = 0; i < vec.size (); i++)
    {
      LOGI ("Load argument %s", (vec[i]));
    }
  global_config = new Configuration (vec.size (), (char **) &(vec[(0)]));

  Configuration & config = *global_config;
  frame = 0;

  LOGI ("Configuration ready");
  // ========= CHECK ARGS =====================

  assert (config.compute_size_ratio > 0);
  assert (config.integration_rate > 0);
  assert (config.volume_size.x > 0);
  assert (config.volume_resolution.x > 0);

  LOGI ("Configuration checked");

  // ========= READER INITIALIZATION  =========

  if (is_file (config.input_file))
    {

      LOGI ("RAW File found\n");
      reader = new RawDepthReader (config.input_file, config.fps,
				   config.blocking_read);

    }
  else
    {
      LOGI ("try OpenNI2 drivers\n");
               reader = new OpenNIDepthReader("",config.fps,config.blocking_read);
       if(!(reader->cameraOpen)) {
           LOGI ("try OpenNI15 drivers\n");
          //This is for openni from a camera
          delete reader;
          reader = new OpenNI15DepthReader("",config.fps,config.blocking_read);
          }

      if(!(reader->cameraOpen)) {
	    LOGI("generation of OpenNI15 and OpeNI2 readers failed\n");
	    delete reader;
	    delete global_config;
	    reader=NULL;
	    return false;
      }
    }

  std::cout.precision (10);
  std::cerr.precision (10);

  init_pose = config.initial_pos_factor * config.volume_size;
  inputSize = reader->getinputSize ();
  std::cerr << "input Size is = " << inputSize.x << "," << inputSize.y
    << std::endl;


  LOGI ("Start memory allocation\n");


  //  =========  BASIC PARAMETERS  (input size / computation size )  =========

  computationSize = make_uint2 (inputSize.x / config.compute_size_ratio,
				inputSize.y / config.compute_size_ratio);
  camera = reader->getK () / config.compute_size_ratio;

  if (config.camera_overrided)
    camera = config.camera / config.compute_size_ratio;

  LOGI("CAMERA CPP= %f %f %f %f." , camera.x, camera.y,camera.z,camera.w);

  //  =========  BASIC BUFFERS  (input / output )  =========

  // Construction Scene reader and input buffer
  inputDepth =
    (uint16_t *) malloc (sizeof (uint16_t) * inputSize.x * inputSize.y);
  raw_rgb = (uchar3 *) malloc (sizeof (uchar3) * inputSize.x * inputSize.y);
  depthRender =
    (uchar4 *) malloc (sizeof (uchar4) * computationSize.x *
		       computationSize.y);
  trackRender =
    (uchar4 *) malloc (sizeof (uchar4) * computationSize.x *
		       computationSize.y);
  volumeRender =
    (uchar4 *) malloc (sizeof (uchar4) * computationSize.x *
		       computationSize.y);


  LOGI ("Run KFusion allocation\n");

  kfusion = new Kfusion (computationSize, config.volume_resolution,
			 config.volume_size, init_pose, config.pyramid);

   LOGI("In  Java_project_pamela_slambench_jni_KFusion_kfusioncppinit\n");

  //kfusion->sender_bind(8800, "10.0.1.23", PREPROCESSING2TRACKING);     // added by Yun
  //kfusion->receiver_bind(8803, RAYCASTING2PREPROCESSING);              //added by Yun

  //kfusion->receiver_bind(8800, PREPROCESSING2TRACKING);              //added by Yun
  //kfusion->sender_bind(8801, "10.0.1.34", TRACKING2INTEGRATION);     // added by Yun


  //kfusion->receiver_bind(8801, TRACKING2INTEGRATION);              //added by Yun
  //kfusion->sender_bind(8802, "10.0.1.28", INTEGRATION2RAYCASTING);     // added by Yun


  kfusion->receiver_bind(8802, INTEGRATION2RAYCASTING);              //added by Yun
  kfusion->sender_bind(8803, "10.0.1.35", RAYCASTING2PREPROCESSING);     // added by Yun

  LOGI ("bindedededededed\n");

  return true;
}

