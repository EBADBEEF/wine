/*
 * NT basis DLL
 *
 * This file contains the Nt* API functions of NTDLL.DLL.
 * In the original ntdll.dll they all seem to just call int 0x2e (down to the NTOSKRNL)
 *
 * Copyright 1996-1998 Marcus Meissner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define NONAMELESSUNION
#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "wine/debug.h"
#include "windef.h"
#include "winternl.h"
#include "ntdll_misc.h"
#include "wine/server.h"
#include "ddk/wdm.h"

WINE_DEFAULT_DEBUG_CHANNEL(ntdll);

/*
 *	Token
 */

/*
 *	Misc
 */

/***********************************************************************
 * RtlIsProcessorFeaturePresent [NTDLL.@]
 */
BOOLEAN WINAPI RtlIsProcessorFeaturePresent( UINT feature )
{
    return feature < PROCESSOR_FEATURE_MAX && user_shared_data->ProcessorFeatures[feature];
}

/******************************************************************************
 * RtlGetNativeSystemInformation [NTDLL.@]
 */
NTSTATUS WINAPI /* DECLSPEC_HOTPATCH */ RtlGetNativeSystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG Length,
	OUT PULONG ResultLength)
{
    FIXME( "semi-stub, SystemInformationClass %#x, SystemInformation %p, Length %#x, ResultLength %p.\n",
            SystemInformationClass, SystemInformation, Length, ResultLength );

    /* RtlGetNativeSystemInformation function is the same as NtQuerySystemInformation on some Win7
     * versions but there are differences for earlier and newer versions, at least:
     *     - HighestUserAddress field for SystemBasicInformation;
     *     - Some information classes are not supported by RtlGetNativeSystemInformation. */
    return NtQuerySystemInformation( SystemInformationClass, SystemInformation, Length, ResultLength );
}

/******************************************************************************
 *  NtCreatePagingFile		[NTDLL.@]
 *  ZwCreatePagingFile		[NTDLL.@]
 */
NTSTATUS WINAPI NtCreatePagingFile(
	PUNICODE_STRING PageFileName,
	PLARGE_INTEGER MinimumSize,
	PLARGE_INTEGER MaximumSize,
	PLARGE_INTEGER ActualSize)
{
    FIXME("(%p %p %p %p) stub\n", PageFileName, MinimumSize, MaximumSize, ActualSize);
    return STATUS_SUCCESS;
}

/******************************************************************************
 *  NtDisplayString				[NTDLL.@]
 *
 * writes a string to the nt-textmode screen eg. during startup
 */
NTSTATUS WINAPI NtDisplayString ( PUNICODE_STRING string )
{
    STRING stringA;
    NTSTATUS ret;

    if (!(ret = RtlUnicodeStringToAnsiString( &stringA, string, TRUE )))
    {
        MESSAGE( "%.*s", stringA.Length, stringA.Buffer );
        RtlFreeAnsiString( &stringA );
    }
    return ret;
}

/******************************************************************************
 *  NtAllocateLocallyUniqueId (NTDLL.@)
 */
NTSTATUS WINAPI NtAllocateLocallyUniqueId(PLUID Luid)
{
    NTSTATUS status;

    TRACE("%p\n", Luid);

    if (!Luid)
        return STATUS_ACCESS_VIOLATION;

    SERVER_START_REQ( allocate_locally_unique_id )
    {
        status = wine_server_call( req );
        if (!status)
        {
            Luid->LowPart = reply->luid.low_part;
            Luid->HighPart = reply->luid.high_part;
        }
    }
    SERVER_END_REQ;

    return status;
}

/******************************************************************************
 *        VerSetConditionMask   (NTDLL.@)
 */
ULONGLONG WINAPI VerSetConditionMask( ULONGLONG dwlConditionMask, DWORD dwTypeBitMask,
                                      BYTE dwConditionMask)
{
    if(dwTypeBitMask == 0)
	return dwlConditionMask;
    dwConditionMask &= 0x07;
    if(dwConditionMask == 0)
	return dwlConditionMask;

    if(dwTypeBitMask & VER_PRODUCT_TYPE)
	dwlConditionMask |= dwConditionMask << 7*3;
    else if (dwTypeBitMask & VER_SUITENAME)
	dwlConditionMask |= dwConditionMask << 6*3;
    else if (dwTypeBitMask & VER_SERVICEPACKMAJOR)
	dwlConditionMask |= dwConditionMask << 5*3;
    else if (dwTypeBitMask & VER_SERVICEPACKMINOR)
	dwlConditionMask |= dwConditionMask << 4*3;
    else if (dwTypeBitMask & VER_PLATFORMID)
	dwlConditionMask |= dwConditionMask << 3*3;
    else if (dwTypeBitMask & VER_BUILDNUMBER)
	dwlConditionMask |= dwConditionMask << 2*3;
    else if (dwTypeBitMask & VER_MAJORVERSION)
	dwlConditionMask |= dwConditionMask << 1*3;
    else if (dwTypeBitMask & VER_MINORVERSION)
	dwlConditionMask |= dwConditionMask << 0*3;
    return dwlConditionMask;
}

/******************************************************************************
 *  NtAccessCheckAndAuditAlarm   (NTDLL.@)
 *  ZwAccessCheckAndAuditAlarm   (NTDLL.@)
 */
NTSTATUS WINAPI NtAccessCheckAndAuditAlarm(PUNICODE_STRING SubsystemName, HANDLE HandleId, PUNICODE_STRING ObjectTypeName,
                                           PUNICODE_STRING ObjectName, PSECURITY_DESCRIPTOR SecurityDescriptor,
                                           ACCESS_MASK DesiredAccess, PGENERIC_MAPPING GenericMapping, BOOLEAN ObjectCreation,
                                           PACCESS_MASK GrantedAccess, PBOOLEAN AccessStatus, PBOOLEAN GenerateOnClose)
{
    FIXME("(%s, %p, %s, %p, 0x%08x, %p, %d, %p, %p, %p), stub\n", debugstr_us(SubsystemName), HandleId,
          debugstr_us(ObjectTypeName), SecurityDescriptor, DesiredAccess, GenericMapping, ObjectCreation,
          GrantedAccess, AccessStatus, GenerateOnClose);

    return STATUS_NOT_IMPLEMENTED;
}
