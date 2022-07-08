//---------------------------------------------------------------------------
#ifndef __apc30_tApcHttpParser_h__
#define __apc30_tApcHttpParser_h__
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ApcHttpParser/iApcHttpParser.h"
//---------------------------------------------------------------------------
#include "http_parser.h" // Используем внутри структуры
//---------------------------------------------------------------------------
#include "ApcNamedItemStore/tApcUnorderedMap.h"
//---------------------------------------------------------------------------
class IApcLog;
//---------------------------------------------------------------------------


/*
  Интерфейс для получения идентификатора хелпа на основании его типа.
*/


class TApcHttpParser : public IApcHttpParser {

public:


  // Создаём объект. (как читатель ответов/ запросов/ или и того и другого)
  TApcHttpParser(enHttpDataType abType);

  ~TApcHttpParser();


  // Запихиваем очередную порцию полученную из сети
  int putNextData(const uint8_t* apBinBuf, int anBufSize);

  // Спрашиваем всё ли завершилось
  bool isMsgComplete() const;


  // Какие данные указывали в констуркторе
  enHttpDataType getDataType() const;


  // ************** Забираем что получилось **************

  int getFieldByID(const std::string& astrFieldID,
                   std::string& astrFieldValue);

  // Если парсили ответ, то можно узнать код
  uint32_t getHttpResultCode() const;

  // Если это был запрос, то можем узнать код метода
  enHttpMethod getHttpRequestMethod() const;

  // Получим копию m_Fields (user сам должен освободить память)
  TApcUnorderedMap<std::string, std::string>* getHeaders() const;

  // Забирем тело запроса
  std::string getBody() const;

  // Забирем URL
  std::string getURL() const;


  // Вероятно позже сделаем ещё получение буфера тела запроса. Пока не знаю
  // параметров

  IApcLog* getLog();


  // ****** функции парсера, но для объекта
  int onField(const char* aszInfo, int anLength);
  int onValue(const char* aszInfo, int anLength);

  int onResStatus(const char* aszInfo, int anLength);

  int onURL(const char* aszInfo, int anLength);

  int onMsgBegin();
  int onHeaderComplete();
  int onChunkComplete();
  int onMsgComplete();

  int onBody(const char* aszInfo, int anLength);


private:

  std::string printFlags(uint8_t abFlags);

  void finalizePreSection(uint8_t abCurrState);


private:

  // То что от библиотеки
  http_parser_settings m_ParsSettings;
  struct http_parser m_Parser;


  IApcLog* m_pLog;

  // Стистика
  int m_nNumOfBytesParced;


  // ***************** данные *******************************
  // Запомним как нас создавали
  enHttpDataType m_bParseType;

  // Всё ли завершено
  bool m_fIsCompelte;

  // Машина состояний, чтобы всякие свойства создавать
  uint8_t m_bState;

  std::string m_strCurrFieldID;
  std::string m_strCurrValueID;
  std::string m_strCurrResStatus;
  std::string m_strCurrBody;
  std::string m_strCurrURL;

  // todo: может положить этот map в std::shared_ptr,
  // чтобы память экономить и не копировать (since C++ 11)
  TApcUnorderedMap<std::string, std::string> m_Fields;
};
#endif
//---------------------------------------------------------------------------