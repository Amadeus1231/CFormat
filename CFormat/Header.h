//---------------------------------------------------------------------------
#ifndef __apc30_tApcHttpParser_h__
#define __apc30_tApcHttpParser_h__
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ApcHttpParser/iApcHttpParser.h"
//---------------------------------------------------------------------------
#include "http_parser.h" // ���������� ������ ���������
//---------------------------------------------------------------------------
#include "ApcNamedItemStore/tApcUnorderedMap.h"
//---------------------------------------------------------------------------
class IApcLog;
//---------------------------------------------------------------------------


/*
  ��������� ��� ��������� �������������� ����� �� ��������� ��� ����.
*/


class TApcHttpParser : public IApcHttpParser {

public:


  // ������ ������. (��� �������� �������/ ��������/ ��� � ���� � �������)
  TApcHttpParser(enHttpDataType abType);

  ~TApcHttpParser();


  // ���������� ��������� ������ ���������� �� ����
  int putNextData(const uint8_t* apBinBuf, int anBufSize);

  // ���������� �� �� �����������
  bool isMsgComplete() const;


  // ����� ������ ��������� � ������������
  enHttpDataType getDataType() const;


  // ************** �������� ��� ���������� **************

  int getFieldByID(const std::string& astrFieldID,
                   std::string& astrFieldValue);

  // ���� ������� �����, �� ����� ������ ���
  uint32_t getHttpResultCode() const;

  // ���� ��� ��� ������, �� ����� ������ ��� ������
  enHttpMethod getHttpRequestMethod() const;

  // ������� ����� m_Fields (user ��� ������ ���������� ������)
  TApcUnorderedMap<std::string, std::string>* getHeaders() const;

  // ������� ���� �������
  std::string getBody() const;

  // ������� URL
  std::string getURL() const;


  // �������� ����� ������� ��� ��������� ������ ���� �������. ���� �� ����
  // ����������

  IApcLog* getLog();


  // ****** ������� �������, �� ��� �������
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

  // �� ��� �� ����������
  http_parser_settings m_ParsSettings;
  struct http_parser m_Parser;


  IApcLog* m_pLog;

  // ��������
  int m_nNumOfBytesParced;


  // ***************** ������ *******************************
  // �������� ��� ��� ���������
  enHttpDataType m_bParseType;

  // �� �� ���������
  bool m_fIsCompelte;

  // ������ ���������, ����� ������ �������� ���������
  uint8_t m_bState;

  std::string m_strCurrFieldID;
  std::string m_strCurrValueID;
  std::string m_strCurrResStatus;
  std::string m_strCurrBody;
  std::string m_strCurrURL;

  // todo: ����� �������� ���� map � std::shared_ptr,
  // ����� ������ ��������� � �� ���������� (since C++ 11)
  TApcUnorderedMap<std::string, std::string> m_Fields;
};
#endif
//---------------------------------------------------------------------------