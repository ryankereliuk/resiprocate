# $Id: Makefile,v 1.7 2004/04/13 17:02:39 jason Exp $

BUILD = ../../build
include $(BUILD)/Makefile.pre

PACKAGES += RESIPROCATE OPENSSL ARES PTHREAD

TARGET_LIBRARY = libdum

TESTPROGRAMS = 

SRC =   \
	BaseCreator.cxx \
	BaseUsage.cxx \
	ClientInviteSession.cxx \
	ClientOutOfDialogReq.cxx \
	ClientPublication.cxx \
	ClientRegistration.cxx \
	ClientSubscription.cxx \
	Dialog.cxx \
	DialogUsageManager.cxx \
	InviteSessionCreator.cxx \
	InviteSession.cxx \
	OutOfDialogReqCreator.cxx \
	PublicationCreator.cxx \
	RegistrationCreator.cxx \
	ServerInviteSession.cxx \
	ServerOutOfDialogReq.cxx \
	ServerPublication.cxx \
	ServerRegistration.cxx \
	ServerSubscription.cxx \
	SubscriptionCreator.cxx \
	UInt64Hash.cxx \

#	DialogSet.cxx DialogSetId.cxx \

include $(BUILD)/Makefile.post
