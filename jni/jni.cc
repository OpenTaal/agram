#define _POSIX_C_SOURCE 200809L

#include <cstddef>
#include <cstdlib>

#include <jni.h>

#include "agram_wc.h"
#include "anagram.h"
#include "anagrams.h"
#include "wc.h"
#include "wl_upgrade.h"
#include "wordlist.h"
#include "words_from.h"

extern "C" JNIEXPORT jbyteArray JNICALL Java_us_achromaticmetaphor_agram_Wordlist_platform(JNIEnv * const env, jclass const cl)
{
  static const jbyte csizes[] = PLATFORM;
  jbyteArray sizes = env->NewByteArray(sizeof(csizes) / sizeof(*csizes));
  if (sizes)
    env->SetByteArrayRegion(sizes, 0, sizeof(csizes) / sizeof(*csizes), csizes);
  return sizes;
}

static void * get_marshalled_pointer_or_null(JNIEnv * const env, jobject const obj, const char * const field)
{
  jfieldID const handlefield = env->GetFieldID(env->GetObjectClass(obj), field, "[B");
  jbyteArray const handle = handlefield ? (jbyteArray) env->GetObjectField(obj, handlefield) : nullptr;
  if (!handle || env->GetArrayLength(handle) != sizeof(void *) / sizeof(jbyte))
    return nullptr;
  void * pointer;
  env->GetByteArrayRegion(handle, 0, sizeof(void *) / sizeof(jbyte), (jbyte *) &pointer);
  return pointer;
}

static void * get_marshalled_pointer(JNIEnv * const env, jobject const obj, const char * const field)
{
  void * pointer = get_marshalled_pointer_or_null(env, obj, field);
  if (!pointer)
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "null or invalid handle");
  return pointer;
}

static int put_marshalled_pointer(JNIEnv * const env, jobject const obj, const char * const field, const void * const pointer)
{
  jfieldID const handlefield = env->GetFieldID(env->GetObjectClass(obj), field, "[B");
  jbyteArray const handle = handlefield ? env->NewByteArray(sizeof(void *) / sizeof(jbyte)) : nullptr;
  if (!handle)
    return 1;

  env->SetByteArrayRegion(handle, 0, sizeof(void *) / sizeof(jbyte), (jbyte *) &pointer);
  env->SetObjectField(obj, handlefield, handle);
  return 0;
}

static void clear_marshalled_pointer(JNIEnv * const env, jobject const obj, const char * const field)
{
  jfieldID const handlefield = env->GetFieldID(env->GetObjectClass(obj), field, "[B");
  if (handlefield)
    env->SetObjectField(obj, handlefield, nullptr);
}

static const struct wordlist * get_wordlist_handle(JNIEnv * const env, jobject const obj)
{
  jfieldID const wordlistField = env->GetFieldID(env->GetObjectClass(obj), "wordlist", "Lus/achromaticmetaphor/agram/Wordlist;");
  jobject const wordlist = wordlistField ? env->GetObjectField(obj, wordlistField) : nullptr;
  const struct wordlist * const wlhandle = wordlist ? (struct wordlist *) get_marshalled_pointer_or_null(env, wordlist, "wordlist_handle") : nullptr;
  if (!wlhandle)
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "null or invalid handle");
  return wlhandle;
}

struct al
{
  JNIEnv * env;
  jmethodID aladd;
  jobject alinstance;
};

static int alinit(struct al * const al, JNIEnv * const env)
{
  al->env = env;
  jclass arraylist = env->FindClass("java/util/ArrayList");
  jmethodID alcon = arraylist ? env->GetMethodID(arraylist, "<init>", "()V") : nullptr;
  al->aladd = alcon ? env->GetMethodID(arraylist, "add", "(Ljava/lang/Object;)Z") : nullptr;
  al->alinstance = al->aladd ? env->NewObject(arraylist, alcon) : nullptr;
  return !al->alinstance;
}

static int stradd(JNIEnv * const env, jobject const obj, jmethodID const mid, jchar const * str, size_t const len)
{
  jstring const s = env->NewString(str, len);
  if (!s)
    return 1;
  env->CallBooleanMethod(obj, mid, s);
  env->DeleteLocalRef(s);
  return 0;
}

static int aladd(jchar const * str, size_t const len, void * const val)
{
  struct al * const al = (struct al *) val;
  return stradd(al->env, al->alinstance, al->aladd, str, len);
}

extern "C" JNIEXPORT jobject JNICALL Java_us_achromaticmetaphor_agram_Anagram_generate(JNIEnv * const env, const jobject obj, const jstring string)
{
  const struct wordlist * const wl = get_wordlist_handle(env, obj);
  if (!wl)
    return nullptr;
  struct al al;
  jchar const * const str = alinit(&al, env) ? nullptr : env->GetStringChars(string, nullptr);
  if (str)
    {
      anagram(wl, str, env->GetStringLength(string), aladd, &al);
      env->ReleaseStringChars(string, str);
    }
  return al.alinstance;
}

extern "C" JNIEXPORT jobject JNICALL Java_us_achromaticmetaphor_agram_WordsFrom_generate(JNIEnv * const env, const jobject obj, const jstring string, const jboolean max)
{
  const struct wordlist * const wl = get_wordlist_handle(env, obj);
  if (!wl)
    return nullptr;
  struct al al;
  jchar const * const str = alinit(&al, env) ? nullptr : env->GetStringChars(string, nullptr);
  if (str)
    {
      words_from(wl, str, env->GetStringLength(string), max, aladd, &al);
      env->ReleaseStringChars(string, str);
    }
  return al.alinstance;
}

extern "C" JNIEXPORT jstring JNICALL Java_us_achromaticmetaphor_agram_Word_pick_1native(JNIEnv * const env, const jclass cl, jobject const wordlist, const jint n)
{
  const struct wordlist * const wl = (struct wordlist *) get_marshalled_pointer(env, wordlist, "wordlist_handle");
  if (!wl)
    return nullptr;
  return env->NewString(wl->words_counts[n].str + wl->strbase, wl->words_counts[n].len);
}

extern "C" JNIEXPORT jint JNICALL Java_us_achromaticmetaphor_agram_Wordlist_get_1nwords(JNIEnv * const env, const jobject obj)
{
  const struct wordlist * const wl = (struct wordlist *) get_marshalled_pointer(env, obj, "wordlist_handle");
  if (!wl)
    return 0;
  return wl->nwords;
}

extern "C" JNIEXPORT jobject JNICALL Java_us_achromaticmetaphor_agram_Anagrams_generate__Ljava_lang_String_2(JNIEnv * const env, const jobject obj, const jstring string)
{
  const struct wordlist * const wl = get_wordlist_handle(env, obj);
  if (!wl)
    return nullptr;
  struct al al;
  jchar const * const str = alinit(&al, env) ? nullptr : env->GetStringChars(string, nullptr);
  if (str)
    {
      anagrams(wl, str, env->GetStringLength(string), aladd, &al);
      env->ReleaseStringChars(string, str);
    }
  return al.alinstance;
}

struct wr : cwlsrc
{
  JNIEnv * env;
  jmethodID wrread;
  jobject wrinstance;
  jstring wrcur;

  int has_next();
  size_t len();
  void get(jchar *);
  wr(JNIEnv *, jobject);
};

int wr::has_next()
{
  if (wrcur)
    env->DeleteLocalRef(wrcur);
  return !!(wrcur = reinterpret_cast<jstring>(env->CallObjectMethod(wrinstance, wrread)));
}

size_t wr::len()
{
  return env->GetStringLength(wrcur);
}

void wr::get(jchar * const out)
{
  env->GetStringRegion(wrcur, 0, len(), out);
}

wr::wr(JNIEnv * const env, jobject const jwords) : env(env), wrinstance(jwords), wrcur(nullptr)
{
  auto const wlreader = env->FindClass("us/achromaticmetaphor/agram/WordlistReader");
  wrread = wlreader ? env->GetMethodID(wlreader, "read", "()Ljava/lang/String;") : nullptr;
}

extern "C" JNIEXPORT void JNICALL Java_us_achromaticmetaphor_agram_Wordlist_loadNullWordlist(JNIEnv * const env, jobject const obj)
{
  static const struct wordlist nullwl = {0};
  put_marshalled_pointer(env, obj, "wordlist_handle", &nullwl);
}

extern "C" JNIEXPORT jboolean JNICALL Java_us_achromaticmetaphor_agram_Wordlist_init__Ljava_lang_String_2Lus_achromaticmetaphor_agram_WordlistReader_2(JNIEnv * const env, jobject const obj, jstring const jfn, jobject const jwords)
{
  struct wr wr(env, jwords);
  const char * const fn = wr.wrread ? env->GetStringUTFChars(jfn, 0) : nullptr;
  const int compile_failed = fn ? wr.compile_wl(fn) : 1;
  auto const wl = compile_failed ? nullptr : new wordlist;
  const int load_failed = wl ? load_wl(wl, fn) : 1;
  if (fn)
    env->ReleaseStringUTFChars(jfn, fn);
  return load_failed || put_marshalled_pointer(env, obj, "wordlist_handle", wl) ? JNI_FALSE : JNI_TRUE;
}

extern "C" JNIEXPORT jboolean JNICALL Java_us_achromaticmetaphor_agram_Wordlist_init__Ljava_lang_String_2(JNIEnv * const env, jobject const obj, jstring const jfn)
{
  const char * const fn = env->GetStringUTFChars(jfn, 0);
  struct wordlist * const wl = fn ? (struct wordlist *) malloc(sizeof(*wl)) : nullptr;
  const int failure = wl ? load_wl(wl, fn) : 1;
  if (fn)
    env->ReleaseStringUTFChars(jfn, fn);
  return failure || put_marshalled_pointer(env, obj, "wordlist_handle", wl) ? JNI_FALSE : JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL Java_us_achromaticmetaphor_agram_Anagrams_uninit(JNIEnv * const env, jobject const obj)
{
  struct agsto * state = (struct agsto *) get_marshalled_pointer_or_null(env, obj, "handle");
  clear_marshalled_pointer(env, obj, "handle");
  anagrams_destroy(state);
  free(state);
}

extern "C" JNIEXPORT jboolean JNICALL Java_us_achromaticmetaphor_agram_Anagrams_init__Ljava_lang_String_2(JNIEnv * const env, jobject const obj, jstring const jstr)
{
  Java_us_achromaticmetaphor_agram_Anagrams_uninit(env, obj);
  const struct wordlist * const wl = get_wordlist_handle(env, obj);
  if (!wl)
    return JNI_FALSE;

  struct agsto * const state = (struct agsto *) malloc(sizeof(*state));
  jchar const * const str = state ? env->GetStringChars(jstr, nullptr) : nullptr;
  int const init_failed = str ? anagrams_init(state, wl, str, env->GetStringLength(jstr)) : 1;
  if (str)
    env->ReleaseStringChars(jstr, str);

  if (init_failed || put_marshalled_pointer(env, obj, "handle", state))
    {
      anagrams_destroy(state);
      free(state);
      return JNI_FALSE;
    }
  else
    return JNI_TRUE;
}

extern "C" JNIEXPORT jobject JNICALL Java_us_achromaticmetaphor_agram_Anagrams_generate__I(JNIEnv * const env, jobject const obj, jint const n)
{
  struct al al;
  if (alinit(&al, env))
    return nullptr;

  struct agsto * state = (struct agsto *) get_marshalled_pointer(env, obj, "handle");
  if (!state)
    return nullptr;

  jint i;
  for (i = 0; i < n; i++)
    {
      size_t const slen = anagrams_single(state);
      if (!slen)
        break;
      if (aladd(state->prefix + 1, slen, &al))
        break;
    }
  return al.alinstance;
}

extern "C" JNIEXPORT jboolean JNICALL Java_us_achromaticmetaphor_agram_Wordlist_upgrade(JNIEnv * const env, jclass const cl, jstring const jfnold, jstring const jfnnew, jbyte const version)
{
  if (!agram_wl_can_upgrade(version))
    return JNI_FALSE;

  const char * const fnold = env->GetStringUTFChars(jfnold, 0);
  const char * const fnnew = fnold ? env->GetStringUTFChars(jfnnew, 0) : nullptr;

  int const failure = fnnew ? agram_wl_upgrade(fnold, fnnew, version) : 1;

  if (fnnew)
    env->ReleaseStringUTFChars(jfnnew, fnnew);
  if (fnold)
    env->ReleaseStringUTFChars(jfnold, fnold);

  return failure ? JNI_FALSE : JNI_TRUE;
}
