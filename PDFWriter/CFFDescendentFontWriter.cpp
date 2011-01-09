#include "CFFDescendentFontWriter.h"
#include "DescendentFontWriter.h"
#include "DictionaryContext.h"
#include "ObjectsContext.h"
#include "CFFEmbeddedFontWriter.h"
#include "FreeTypeFaceWrapper.h"
#include "Trace.h"

CFFDescendentFontWriter::CFFDescendentFontWriter(void)
{
}

CFFDescendentFontWriter::~CFFDescendentFontWriter(void)
{
}

static bool sEncodedGlypsSort(const UIntAndGlyphEncodingInfo& inLeft, const UIntAndGlyphEncodingInfo& inRight)
{
	return inLeft.first < inRight.first;
}

static const string scCIDFontType0C = "CIDFontType0C";
static const char* scType1 = "Type 1";
EStatusCode CFFDescendentFontWriter::WriteFont(	ObjectIDType inDecendentObjectID, 
														const string& inFontName,
														FreeTypeFaceWrapper& inFontInfo,
														const UIntAndGlyphEncodingInfoVector& inEncodedGlyphs,
														ObjectsContext* inObjectsContext)
{
	DescendentFontWriter descendentFontWriter;
	string subsetFontName;

	EStatusCode status = descendentFontWriter.WriteFont(inDecendentObjectID,inFontName,inFontInfo,inEncodedGlyphs,inObjectsContext,this,subsetFontName);

	if(eFailure == status)
		return status;

	// Logically speaking, i shouldn't be getting to CID writing
	// if in type 1. at least, this is the current assumption, since
	// i don't intend to support type 1 CIDs, but just regular type 1s.
	// as such - fail if got here for type 1
	const char* fontType = inFontInfo.GetTypeString();
	if(strcmp(scType1,fontType) == 0)
	{
		TRACE_LOG1("CFFDescendentFontWriter::WriteFont, Exception. identified type1 font when writing CFF CID font, font name - %s. type 1 CIDs are not supported.",inFontName.c_str());
		return eFailure;
	}


	CFFEmbeddedFontWriter embeddedFontWriter;
	UIntAndGlyphEncodingInfoVector encodedGlyphs = inEncodedGlyphs;
	UIntVector orderedGlyphs;
	UShortVector cidMapping;
	
	sort(encodedGlyphs.begin(),encodedGlyphs.end(),sEncodedGlypsSort);	

	for(UIntAndGlyphEncodingInfoVector::const_iterator it = encodedGlyphs.begin(); 
		it != encodedGlyphs.end(); 
		++it)
	{
		orderedGlyphs.push_back(it->first);
		cidMapping.push_back(it->second.mEncodedCharacter);
	}
	return embeddedFontWriter.WriteEmbeddedFont(inFontInfo,
												orderedGlyphs,
												scCIDFontType0C,
												mEmbeddedFontFileObjectID,
												subsetFontName,
												inObjectsContext,
												&cidMapping);
}

static const string scCIDFontType0 = "CIDFontType0";

void CFFDescendentFontWriter::WriteSubTypeValue(DictionaryContext* inDescendentFontContext)
{
	inDescendentFontContext->WriteNameValue(scCIDFontType0);
}

void CFFDescendentFontWriter::WriteAdditionalKeys(DictionaryContext* inDescendentFontContext)
{
	// do nothing
}

static const string scFontFile3 = "FontFile3";
void CFFDescendentFontWriter::WriteFontFileReference(DictionaryContext* inDescriptorContext,
													ObjectsContext* inObjectsContext)
{
	// FontFile3
	inDescriptorContext->WriteKey(scFontFile3);
	mEmbeddedFontFileObjectID = inObjectsContext->GetInDirectObjectsRegistry().AllocateNewObjectID();
	inDescriptorContext->WriteObjectReferenceValue(mEmbeddedFontFileObjectID);	
}