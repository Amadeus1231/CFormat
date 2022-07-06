//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "tApcHttpParser.h"
//---------------------------------------------------------------------------
#include "ApcLog/tApcLogMacros.h"
//---------------------------------------------------------------------------
#include "ApcLog/tLXMemLeaks.h"

//---------------------------------------------------------------------------


int on_msg_begin(http_parser* apParser)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onMsgBegin();
}

//---------------------------------------------------------------------------


int on_msg_complete(http_parser* apParser)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onMsgComplete();
}

//---------------------------------------------------------------------------


int on_header_complete(http_parser* apParser)
{

    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onHeaderComplete();
}

//---------------------------------------------------------------------------


int on_chunk_header(http_parser* apParser)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    IApcLog* pLog = pObj->getLog();

    __L_ANY(pLog, "on_chunk_header()");
    return 0;
}

//---------------------------------------------------------------------------


int on_chunk_complete(http_parser* apParser)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onChunkComplete();
}

//---------------------------------------------------------------------------


int on_url(http_parser* apParser, const char* aszInfo, size_t anLength)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onURL(aszInfo, anLength);
}

//---------------------------------------------------------------------------


int on_field(http_parser* apParser, const char* aszInfo, size_t anLength)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onField(aszInfo, anLength);
}

//---------------------------------------------------------------------------


int on_value(http_parser* apParser, const char* aszInfo, size_t anLength)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onValue(aszInfo, anLength);
}

//---------------------------------------------------------------------------


int on_ResStatus(http_parser* apParser, const char* aszInfo, size_t anLength)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onResStatus(aszInfo, anLength);
}

//---------------------------------------------------------------------------


int on_Body(http_parser* apParser, const char* aszInfo, size_t anLength)
{
    TApcHttpParser* pObj = (TApcHttpParser*)(apParser->data);
    if (!pObj) {
        // некуда логировать :(
        return -33;
    }

    return pObj->onBody(aszInfo, anLength);
}

//---------------------------------------------------------------------------


// Создаём объект. (как читатель ответов/ запросов/ или и того и другого)
TApcHttpParser::TApcHttpParser(
    enHttpDataType abType)
    : m_pLog(IApcLog::getLog(__thisName))
    , m_nNumOfBytesParced(0)
    , m_bParseType(abType)
    , m_fIsCompelte(false)
    , m_bState(0)
{
    ::ZeroMemory(&m_ParsSettings, sizeof(http_parser_settings));

    m_ParsSettings.on_message_begin = on_msg_begin;
    m_ParsSettings.on_message_complete = on_msg_complete;
    m_ParsSettings.on_headers_complete = on_header_complete;

    m_ParsSettings.on_status = on_ResStatus;

    m_ParsSettings.on_header_field = on_field;
    m_ParsSettings.on_header_value = on_value;

    m_ParsSettings.on_url = on_url;
    m_ParsSettings.on_body = on_Body;

    m_ParsSettings.on_chunk_header = on_chunk_header;
    m_ParsSettings.on_chunk_complete = on_chunk_complete;

    http_parser_init(&m_Parser, static_cast<http_parser_type>(abType));
    m_Parser.data = this;
};

//---------------------------------------------------------------------------


TApcHttpParser::~TApcHttpParser() {};

//---------------------------------------------------------------------------


// Запихиваем очередную порцию полученную из сети
int TApcHttpParser::putNextData(const uint8_t* apBinBuf, int anBufSize)
{
    int nParsed = http_parser_execute(
        &m_Parser,
        &m_ParsSettings,
        reinterpret_cast<const char*>(apBinBuf),
        anBufSize);
    if (nParsed != anBufSize) {
        __L_BAD(m_pLog, "Num of processed not equal to putted. Income bytes: " + apc::toStr<int>(anBufSize) + ", parsed: " + apc::toStr<int>(nParsed));
        return -33;
    }

    m_nNumOfBytesParced += nParsed;

    return 0;
};

//---------------------------------------------------------------------------

// Спрашиваем всё ли завершилось
bool TApcHttpParser::isMsgComplete() const
{
    return m_fIsCompelte;
};

//---------------------------------------------------------------------------

// ************** Забираем что получилось **************

int TApcHttpParser::getFieldByID(const std::string& astrFieldID, std::string& astrFieldValue)
{
    return m_Fields.get(apc::upperCase(astrFieldID), astrFieldValue);
};

//---------------------------------------------------------------------------


// Если парсили ответ, то можно узнать код
uint32_t TApcHttpParser::getHttpResultCode() const
{
    return m_Parser.status_code;
};

//---------------------------------------------------------------------------

// Возвращаем копию m_Fields
TApcUnorderedMap<std::string, std::string>* TApcHttpParser::getHeaders() const
{
    std::unique_ptr<TApcUnorderedMap<std::string, std::string>> spHeaders(new TApcUnorderedMap<std::string, std::string>());
    std::string strKey, strVal;
    for (
        int nRes = m_Fields.getFirstPair(strKey, strVal);
        !nRes;
        nRes = m_Fields.getNextPair(strKey, strVal))
    {
        spHeaders->insert(strKey, strVal);
    }
    return spHeaders.release();
};

//---------------------------------------------------------------------------

// Забирем тело запроса
std::string TApcHttpParser::getBody() const
{
    return m_strCurrBody;
};

//---------------------------------------------------------------------------


// Если это был запрос, то можем узнать код метода
enHttpMethod TApcHttpParser::getHttpRequestMethod() const
{
    return static_cast<enHttpMethod>(m_Parser.method);
};

//---------------------------------------------------------------------------


IApcLog* TApcHttpParser::getLog()
{
    return m_pLog;
};

//---------------------------------------------------------------------------


int TApcHttpParser::onField(const char* aszInfo, int anLength)
{

    if (m_bState != 1) {
        finalizePreSection(m_bState);
    }


    // Если это начало:
    if (m_bState != 1)
    {
        if (m_strCurrFieldID != "")
        {
            __L_BAD(m_pLog, "m_strCurrFieldID is not nullptr. Value: " + m_strCurrFieldID);
        }
        m_strCurrFieldID = std::string(aszInfo, anLength);
    }
    else
    {
        m_strCurrFieldID += std::string(aszInfo, anLength);
    }

    // Типа начали читать ID значения
    m_bState = 1;


    return 0;
};

//---------------------------------------------------------------------------

int TApcHttpParser::onValue(const char* aszInfo, int anLength)
{

    // Может быть сразу или может быть продолжение "имени"
    if (m_bState != 2) {
        finalizePreSection(m_bState);
    }


    if (m_bState != 2)
    {
        if (m_strCurrValueID != "")
        {
            __L_BAD(m_pLog, "m_strCurrFieldID is not nullptr. Value: " + m_strCurrFieldID);
        }
        m_strCurrValueID = std::string(aszInfo, anLength);
    }
    else
    {
        m_strCurrValueID += std::string(aszInfo, anLength);
    }


    // Типа начали читать само значение
    m_bState = 2;


    return 0;
};

//---------------------------------------------------------------------------


int TApcHttpParser::onResStatus(const char* aszInfo, int anLength)
{

    // Если мы до этого были где-то ещё, то нужно завершить "другие":
    if (m_bState != 3) {
        finalizePreSection(m_bState);
    }


    // Если это начало:
    if (m_bState != 3)
    {
        if (m_strCurrResStatus != "")
        {
            __L_BAD(m_pLog, "m_strCurrResStatus is not nullptr. Value: " + m_strCurrResStatus);
        }

        m_strCurrResStatus = std::string(aszInfo, anLength);
    }
    else //Если это продолжение:
    {
        m_strCurrResStatus += std::string(aszInfo, anLength);
    }

    // Типа начали читать ID значения
    m_bState = 3;


    return 0;
};

//---------------------------------------------------------------------------


void TApcHttpParser::finalizePreSection(BYTE abCurrState)
{
    switch (abCurrState)
    {

    case 0:
    {
        // если это начало, то ничего делать не надо.
    } break;


    case 1:
    {
        //__L_TRK(m_pLog, "Last propID: " + m_strCurrFieldID);

    } break;

    case 2:
    {
        //__L_TRK(m_pLog, "Last propValue: " + m_strCurrValueID);


        // Всё будем хранить в "верхнем регистре", чтобы потом искать "без регистра"
        std::string strUpperCaseField = apc::upperCase(m_strCurrFieldID);
        int nResult = m_Fields.insert(strUpperCaseField, m_strCurrValueID);
        if (nResult)
        {
            if (strUpperCaseField == "VARY") { // 'Vary' exception
                if (m_Fields.set(strUpperCaseField, m_strCurrValueID)) {
                    __L_BADR(m_pLog,
                             "Error in updatePair() for: 'VARY' value: '" + m_strCurrValueID + "'",
                             nResult);
                }
            }
            else {
                __L_BADR(m_pLog,
                         "Error in addPair() for: " + strUpperCaseField + ", value: " + m_strCurrValueID + ". Check for duplicates",
                         nResult);
            }
        }

        // Чистим всё для возможно следующих значений:
        m_strCurrFieldID.clear();
        m_strCurrValueID.clear();

    } break;

    case 3:
    {
        // Не будем спамить.
        __L_ANY(m_pLog, "Last strResultStatus: " + m_strCurrResStatus);

        // "Чистить" старый результат не будем. Их не бывает "много"
        // m_strCurrResStatus = "";
    } break;

    case 4:
    {
        __L_ANY(m_pLog, "body complete: " + m_strCurrBody);

        // "Чистить" старый результат не будем. Их не бывает "много"
        // m_strCurrResStatus = "";
    } break;


    case 5:
    {
        __L_ANY(m_pLog, "URL complete: " + m_strCurrURL);

        // "Чистить" старый результат не будем. Их не бывает "много"
        // m_strCurrResStatus = "";
    } break;


    default:
        __L_BAD(m_pLog, "Unexpected state. Value: " + apc::toStr<BYTE>(abCurrState));
        break;
    };
};

//---------------------------------------------------------------------------


int TApcHttpParser::onHeaderComplete()
{
    __L_ANY(m_pLog, "onHeaderComplete()");


    // Всё закрываем:
    finalizePreSection(m_bState);
    m_bState = 0;

    // Если хидер закончился и длина доп данных 0, то выходим:


    if (m_Parser.flags & F_CONTENTLENGTH) {
        __L_ANY(m_pLog, "Size of content payload: " + apc::toStr<uint64_t>(m_Parser.content_length))
    }
    else {
        __L_ANY(m_pLog, "No contents payload");
    }

    // Если есть длина в заголовке, или же проставлен флаг chuncked, то продолжаем,
    // иначе - выходим
    if (!(m_Parser.flags & F_CONTENTLENGTH) && !(m_Parser.flags & F_CHUNKED))
    {
        if (m_bParseType == enHttpDataType::enHttpDataType_Response) // на "ответ" будем считать что мы всё.
        {
            m_fIsCompelte = true;
        }
    }

    return 0;
};

//---------------------------------------------------------------------------


int TApcHttpParser::onMsgBegin()
{
    // Всё приводим в исходное состояние:

    // Всё закрываем:
    finalizePreSection(m_bState);
    m_bState = 0;

    m_nNumOfBytesParced = 0;
    m_fIsCompelte = false;

    m_strCurrFieldID = m_strCurrValueID = m_strCurrResStatus = m_strCurrBody = m_strCurrURL = "";
    m_Fields.clear();

    return 0;
}

//---------------------------------------------------------------------------


int TApcHttpParser::onMsgComplete()
{
    __L_ANY(m_pLog, "onMsgComplete()");

    // Всё закрываем:
    finalizePreSection(m_bState);
    m_bState = 0;


    // Теперь и на запрос и на ответ здесь будет нормально:
    // if(m_bParseType == 0) // на "запрос" будем считать что мы всё.
    {
        m_fIsCompelte = true;
    }

    return 0;
}

//---------------------------------------------------------------------------

int TApcHttpParser::onChunkComplete()
{
    __L_ANY(m_pLog, "onChunkComplete()");


    // Всё закрываем:
    finalizePreSection(m_bState);

    // на "запрос" будем считать что мы всё.
    if (m_bParseType == enHttpDataType::enHttpDataType_Request) {
        m_fIsCompelte = true;
        m_bState = 0;
    }

    return 0;
};

//---------------------------------------------------------------------------


int TApcHttpParser::onBody(const char* aszInfo, int anLength)
{
    __L_STACK__

    // Если мы до этого были где-то ещё, то нужно завершить "другие":
    if (m_bState != 4) {
        finalizePreSection(m_bState);
    }


    // Если это начало:
    if (m_bState != 4)
    {
        if (m_strCurrBody != "")
        {
            __L_BAD(m_pLog, "m_strCurrBody is not nullptr. Value: " + m_strCurrBody);
        }
        m_strCurrBody = std::string(aszInfo, anLength);
    }
    else //Если это продолжение:
    {
        m_strCurrBody += std::string(aszInfo, anLength);
    }

    // Типа начали читать Body
    m_bState = 4;

    return 0;
};

//---------------------------------------------------------------------------


int TApcHttpParser::onURL(const char* aszInfo, int anLength)
{
    __L_STACK__

    // Если мы до этого были где-то ещё, то нужно завершить "другие":
    if (m_bState != 5) {
        finalizePreSection(m_bState);
    }


    // Если это начало:
    if (m_bState != 5)
    {
        if (m_strCurrURL != "")
        {
            __L_BAD(m_pLog, "m_strCurrURL is not nullptr. Value: " + m_strCurrURL);
        }

        m_strCurrURL = std::string(aszInfo, anLength);
    }
    else //Если это продолжение:
    {
        m_strCurrURL += std::string(aszInfo, anLength);
    }

    // Типа начали читать Body
    m_bState = 5;

    return 0;
};

//---------------------------------------------------------------------------


// Какие данные указывали в констуркторе
enHttpDataType TApcHttpParser::getDataType() const
{
    return m_bParseType;
};

//---------------------------------------------------------------------------


// Забирем URL
std::string TApcHttpParser::getURL() const
{
    return m_strCurrURL;
};

//---------------------------------------------------------------------------


std::string TApcHttpParser::printFlags(BYTE abFlags)
{
    std::string strResult;

    // TODO: добавить вывод оставльных флагов? Или это вообще нигде не используется?

    if (abFlags & F_CONTENTLENGTH) {
        strResult += ", CONTENTLENGTH";
    }

    if (abFlags & F_CHUNKED) {
        strResult += ", F_CHUNKED";
    }

    return strResult;
}

//---------------------------------------------------------------------------