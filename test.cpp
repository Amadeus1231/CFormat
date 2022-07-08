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
