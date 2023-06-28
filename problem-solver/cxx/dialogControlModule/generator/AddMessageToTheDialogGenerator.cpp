#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-agents-common/utils/GenerationUtils.hpp"
#include "keynodes/Keynodes.hpp"
#include "AddMessageToTheDialogGenerator.hpp"
#include "keynodes/MessageKeynodes.hpp"

using namespace utils;
using namespace std;

namespace dialogControlModule
{
AddMessageToTheDialogGenerator::AddMessageToTheDialogGenerator(ScMemoryContext * context)
  : context(context)
{
}

void AddMessageToTheDialogGenerator::addMessageToDialog(ScAddr const & dialogAddr, ScAddr const & messageAddr)
{
  auto scTemplate = std::make_unique<ScTemplate>();
  ScAddr lastMessageAddr;

  ScIterator5Ptr iterator5Ptr = context->Iterator5(
      dialogAddr,
      ScType::EdgeAccessConstPosPerm,
      ScType::NodeConst,
      ScType::EdgeAccessConstPosTemp,
      commonModule::Keynodes::rrel_last);

  if (iterator5Ptr->Next())
    lastMessageAddr = iterator5Ptr->Get(2);

  if (lastMessageAddr.IsValid())
  {
    ScIterator5Ptr it5 = context->Iterator5(
        dialogAddr,
        ScType::EdgeAccessConstPosPerm,
        lastMessageAddr,
        ScType::EdgeAccessConstPosTemp,
        commonModule::Keynodes::rrel_last);

    if (it5->Next())
      context->EraseElement(it5->Get(3));

    scTemplate = createNotFirstMessageInDialogTemplate(dialogAddr, lastMessageAddr, messageAddr);
  }
  else
    scTemplate = createFirstMessageInDialogTemplate(dialogAddr, messageAddr);

  ScTemplateGenResult genResult;
  if (context->HelperGenTemplate(*scTemplate, genResult) == SC_FALSE)
    throw std::runtime_error("Unable to generate structure for next dialog message.");
}

std::unique_ptr<ScTemplate> AddMessageToTheDialogGenerator::createNotFirstMessageInDialogTemplate(
    ScAddr const & dialogAddr,
    ScAddr const & lastMessageAddr,
    ScAddr const & messageAddr)
{
  string const NEXT_MESSAGE_ARC_ALIAS = "_next_message_arc";

  ScAddr messageEdge;
  ScIterator3Ptr iterator3Ptr = context->Iterator3(dialogAddr, ScType::EdgeAccessConstPosPerm, lastMessageAddr);
  if (iterator3Ptr->Next())
    messageEdge = iterator3Ptr->Get(1);

  auto scTemplate = std::make_unique<ScTemplate>();
  scTemplate->TripleWithRelation(
      dialogAddr,
      ScType::EdgeAccessVarPosPerm >> NEXT_MESSAGE_ARC_ALIAS,
      messageAddr,
      ScType::EdgeAccessVarPosTemp,
      commonModule::Keynodes::rrel_last);
  scTemplate->TripleWithRelation(
      messageEdge,
      ScType::EdgeDCommonVar,
      NEXT_MESSAGE_ARC_ALIAS,
      ScType::EdgeAccessVarPosPerm,
      MessageKeynodes::nrel_message_sequence);

  return scTemplate;
}

std::unique_ptr<ScTemplate> AddMessageToTheDialogGenerator::createFirstMessageInDialogTemplate(
    ScAddr const & dialogAddr,
    ScAddr const & messageAddr)
{
  auto scTemplate = std::make_unique<ScTemplate>();
  scTemplate->TripleWithRelation(
      dialogAddr,
      ScType::EdgeAccessVarPosPerm,
      messageAddr,
      ScType::EdgeAccessVarPosPerm,
      scAgentsCommon::CoreKeynodes::rrel_1);
  scTemplate->TripleWithRelation(
      dialogAddr,
      ScType::EdgeAccessVarPosPerm,
      messageAddr,
      ScType::EdgeAccessVarPosTemp,
      commonModule::Keynodes::rrel_last);
  return scTemplate;
}

}  // namespace dialogControlModule
