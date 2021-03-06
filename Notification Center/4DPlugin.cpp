/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : Notification Center
 #	author : miyako
 #	2016/01/27
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

#define CALLBACK_IN_NEW_PROCESS 0
#define CALLBACK_SLEEP_TIME 59

#define MAX_USERINFO_LENGTH 1000

std::mutex globalMutex; /* CALLBACK_EVENT_IDS */
std::mutex globalMutex0;/* shouldPresentNotification */
std::mutex globalMutex1;/* for MONITOR_PROCESS_ID */
std::mutex globalMutex2;/* for LISTENER_METHOD */
std::mutex globalMutex3;/* PROCESS_SHOULD_TERMINATE */
std::mutex globalMutex4;/* PROCESS_SHOULD_RESUME */

#pragma mark -

@interface Listener : NSObject <NSUserNotificationCenterDelegate>
{
	NSMutableArray *eventContexts;
}
- (id)init;
- (void)dealloc;
- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification;
- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification;
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification;
- (void)setEventContext:(NSUserNotification *)notification notificationType:(NSString *)notificationType activationType:(NSString *)activationType;
- (void)call:(event_id_t)event;
- (NSDictionary *)getOldestEventContext;
- (void)removeOldestEventContext;
@end

namespace UN
{
    Listener *listener = nil;
		
    //constants
    process_name_t MONITOR_PROCESS_NAME = (PA_Unichar *)"$\0U\0s\0e\0r\0 \0N\0o\0t\0i\0f\0i\0c\0a\0t\0i\0o\0n\0 \0L\0i\0s\0t\0e\0n\0e\0r\0\0\0";
    process_number_t MONITOR_PROCESS_ID = 0;
    process_stack_size_t MONITOR_PROCESS_STACK_SIZE = 0;
    
    //context management
    std::vector<event_id_t>CALLBACK_EVENT_IDS;
    BOOL shouldPresentNotification = YES;

    //callback management
    C_TEXT LISTENER_METHOD;
    bool PROCESS_SHOULD_TERMINATE = false;
    bool PROCESS_SHOULD_RESUME = false;
	
    PA_Unistring setUnistringVariable(PA_Variable *v, NSString *s)
    {
        C_TEXT t;
        t.setUTF16String(s);
        PA_Unistring u = PA_CreateUnistring((PA_Unichar *)t.getUTF16StringPtr());
        PA_SetStringVariable(v, &u);
        return u;
    }

    NSString *getDictionaryString(NSDictionary *dictionary)
    {
        return dictionary ? [dictionary objectForKey:@"userInfo"]: @"";
    }
	
    NSDictionary *makeStringDictionary(NSString *string)
    {
        
        NSDictionary *dictionary;
        if((string) && ([string length] < MAX_USERINFO_LENGTH))
        {
            dictionary = [NSDictionary dictionaryWithObject:string forKey:@"userInfo"];
        }else
        {
            dictionary = [NSDictionary dictionaryWithObject:@"" forKey:@"userInfo"];
        }
        
        return dictionary;
    }
	
    NSUserNotification *createNotification(sLONG_PTR *pResult, PackagePtr pParams)
    {
        C_TEXT Param1;
        C_TEXT Param2;
        C_TEXT Param3;
        C_TEXT Param4;
        C_TEXT Param6;
        C_TEXT Param5;
        C_TEXT Param7;
        
        Param1.fromParamAtIndex(pParams, 1);
        Param2.fromParamAtIndex(pParams, 2);
        Param3.fromParamAtIndex(pParams, 3);
        Param4.fromParamAtIndex(pParams, 4);
        Param5.fromParamAtIndex(pParams, 5);
        Param6.fromParamAtIndex(pParams, 6);
        Param7.fromParamAtIndex(pParams, 7);
        
        NSUserNotification *notification = [[NSUserNotification alloc]init];
        
        NSString *title = Param1.copyUTF16String();
        notification.title = title;
        [title release];
        
        NSString *subtitle = Param2.copyUTF16String();
        notification.subtitle = subtitle;
        [subtitle release];
        
        NSString *informativeText = Param3.copyUTF16String();
        notification.informativeText = informativeText;
        [informativeText release];
        
        if(!Param4.getUTF16Length()){
            notification.soundName = nil;
        }else{
            NSString *soundName = Param4.copyUTF16String();
            if([soundName isEqualToString:@"__DEFAULT__"]){
                notification.soundName = NSUserNotificationDefaultSoundName;
            }else{
                notification.soundName = soundName;
            }
            [soundName release];
        }
        
        NSString *userInfo = Param5.copyUTF16String();
        notification.userInfo = UN::makeStringDictionary(userInfo);
        [userInfo release];
        
        if(!Param6.getUTF16Length()){
            notification.hasActionButton = NO;
        }else{
            notification.hasActionButton = YES;
            NSString *actionButtonTitle = Param6.copyUTF16String();
            notification.actionButtonTitle = actionButtonTitle;
            [actionButtonTitle release];
        }
        
        if(Param7.getUTF16Length()){
            NSString *otherButtonTitle = Param7.copyUTF16String();
            notification.otherButtonTitle = otherButtonTitle;
            [otherButtonTitle release];
        }
        
        return notification;
    }
}

@implementation Listener

- (id)init
{
	if(!(self = [super init])) return self;
	
	if(NSFoundationVersionNumber >= NSFoundationVersionNumber10_8)
	{
		NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
		center.delegate = self;
	}
	
	eventContexts = [[NSMutableArray alloc]init];
	
	return self;
}

- (void)dealloc
{
	[[[NSWorkspace sharedWorkspace] notificationCenter]removeObserver:self];
	
//    std::lock_guard<std::mutex> lock(globalMutex);
	
	UN::CALLBACK_EVENT_IDS.clear();
	
	[eventContexts release];
	
	[super dealloc];
}

- (void)removeOldestEventContext
{
	if([eventContexts count])
	{
		[eventContexts removeObjectAtIndex:0];
	}
}

- (NSDictionary *)getOldestEventContext
{
	if([eventContexts count])
	{
		return [eventContexts objectAtIndex:0];
	}
	return nil;
}

- (void)setEventContext:(NSUserNotification *)notification notificationType:(NSString *)notificationType activationType:(NSString *)activationType
{
	NSString *userInfo = UN::getDictionaryString(notification.userInfo);
	
	NSDictionary *eventContext = [[NSDictionary alloc]initWithObjectsAndKeys:
    notification.title, @"title",
		notification.subtitle, @"subtitle",
		notification.informativeText, @"informativeText",
		notificationType, @"notificationType",
		activationType, @"activationType",
		userInfo, @"userInfo", nil];
	
	[eventContexts addObject:eventContext];
	
	[eventContext release];
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification
{
	[self setEventContext:notification notificationType:@"DeliverNotification" activationType:@""];
	
	[self call:1];
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
		switch(notification.activationType)
		{
			case NSUserNotificationActivationTypeNone:
				[self setEventContext:notification notificationType:@"ActivateNotification" activationType:@"ActivationTypeNone"];
				break;
			case NSUserNotificationActivationTypeContentsClicked:
				[self setEventContext:notification notificationType:@"ActivateNotification" activationType:@"ContentsClicked"];
				break;	
			case NSUserNotificationActivationTypeActionButtonClicked:
				[self setEventContext:notification notificationType:@"ActivateNotification" activationType:@"ActionButtonClicked"];
				break;		
			default:
				[self setEventContext:notification notificationType:@"ActivateNotification" activationType:@""];
				break;
		}
		[self call:2];
}

- (void)call:(event_id_t)event
{
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex);
        
        UN::CALLBACK_EVENT_IDS.push_back(event);
    }
    
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex4);
        
        UN::PROCESS_SHOULD_RESUME = true;
    }
    
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
	return UN::shouldPresentNotification;
}

@end

#pragma mark -

void generateUuid(C_TEXT &returnValue)
{
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1080
    returnValue.setUTF16String([[[NSUUID UUID]UUIDString]stringByReplacingOccurrencesOfString:@"-" withString:@""]);
#else
    CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
    NSString *uuid_str = (NSString *)CFUUIDCreateString(kCFAllocatorDefault, uuid);
    returnValue.setUTF16String([uuid_str stringByReplacingOccurrencesOfString:@"-" withString:@""]);
#endif
}

#pragma mark -

bool IsProcessOnExit()
{
    C_TEXT name;
    PA_long32 state, time;
    PA_GetProcessInfo(PA_GetCurrentProcessNumber(), name, &state, &time);
    CUTF16String procName(name.getUTF16StringPtr());
    CUTF16String exitProcName((PA_Unichar *)"$\0x\0x\0\0\0");
    return (!procName.compare(exitProcName));
}

void OnStartup()
{

}

void OnCloseProcess()
{
	if(IsProcessOnExit())
	{
		listenerLoopFinish();
	}
}

void listenerLoop()
{
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex3);
        
        UN::PROCESS_SHOULD_TERMINATE = false;
    }
    
    while(!PA_IsProcessDying())
    {
        PA_YieldAbsolute();
        
        bool PROCESS_SHOULD_RESUME;
        bool PROCESS_SHOULD_TERMINATE;
        
        if(1)
        {
            PROCESS_SHOULD_RESUME = UN::PROCESS_SHOULD_RESUME;
            PROCESS_SHOULD_TERMINATE = UN::PROCESS_SHOULD_TERMINATE;
        }
        
        if(PROCESS_SHOULD_RESUME)
        {
            size_t EVENT_IDS;
            
            if(1)
            {
                std::lock_guard<std::mutex> lock(globalMutex);
                
                EVENT_IDS = UN::CALLBACK_EVENT_IDS.size();
            }
            
            while(EVENT_IDS)
            {
                PA_YieldAbsolute();
                
                if(CALLBACK_IN_NEW_PROCESS)
                {
                    C_TEXT processName;
                    generateUuid(processName);
                    PA_NewProcess((void *)listenerLoopExecuteMethod,
                                  UN::MONITOR_PROCESS_STACK_SIZE,
                                  (PA_Unichar *)processName.getUTF16StringPtr());
                }else
                {
                    listenerLoopExecuteMethod();
                }
                
                if(PROCESS_SHOULD_TERMINATE)
                    break;
                
                if(1)
                {
                    std::lock_guard<std::mutex> lock(globalMutex);
                    
                    EVENT_IDS = UN::CALLBACK_EVENT_IDS.size();
                    PROCESS_SHOULD_TERMINATE = UN::PROCESS_SHOULD_TERMINATE;
                }
            }

            if(1)
            {
                std::lock_guard<std::mutex> lock(globalMutex4);
                
                UN::PROCESS_SHOULD_RESUME = false;
            }
            
        }else
        {
            PA_PutProcessToSleep(PA_GetCurrentProcessNumber(), CALLBACK_SLEEP_TIME);
        }
        
        if(1)
        {
            PROCESS_SHOULD_TERMINATE = UN::PROCESS_SHOULD_TERMINATE;
        }
        
        if(PROCESS_SHOULD_TERMINATE)
            break;
        
    }
    
    
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex2);
        
        UN::LISTENER_METHOD.setUTF16String((PA_Unichar *)"\0\0", 0);
    }
    
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex1);
        
        UN::MONITOR_PROCESS_ID = 0;
    }
    
    PA_RunInMainProcess((PA_RunInMainProcessProcPtr)listener_end, NULL);
    
    PA_KillProcess();
}

void listener_start()
{
	if(!UN::listener)
	{
		UN::listener = [[Listener alloc]init];
	}
	
}

void listener_end()
{
	/* must do this in main process */
	[UN::listener release];
	UN::listener = nil;
}

void listenerLoopStart()
{
	if(!UN::MONITOR_PROCESS_ID)
	{
        std::lock_guard<std::mutex> lock(globalMutex1);
        
        UN::MONITOR_PROCESS_ID = PA_NewProcess((void *)listenerLoop,
                                               UN::MONITOR_PROCESS_STACK_SIZE,
                                               UN::MONITOR_PROCESS_NAME);
	}
}

void listenerLoopFinish()
{
	if(UN::MONITOR_PROCESS_ID)
	{
        if(1)
        {
            std::lock_guard<std::mutex> lock(globalMutex3);
            
            UN::PROCESS_SHOULD_TERMINATE = true;
        }
        
        PA_YieldAbsolute();

        if(1)
        {
            std::lock_guard<std::mutex> lock(globalMutex4);
            
            UN::PROCESS_SHOULD_RESUME = true;
        }
	}
}

void listenerLoopExecute()
{
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex3);
        
        UN::PROCESS_SHOULD_TERMINATE = false;
    }
    
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex4);
        
        UN::PROCESS_SHOULD_RESUME = true;
    }
}

void listenerLoopExecuteMethod()
{
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex);
        
        std::vector<event_id_t>::iterator e;
        
        e = UN::CALLBACK_EVENT_IDS.begin();
     
        UN::CALLBACK_EVENT_IDS.erase(e);
    }
    
	NSDictionary *eventContext = [UN::listener getOldestEventContext];
	
	@autoreleasepool
	{
		if(eventContext)
		{
			NSString *title = [eventContext objectForKey:@"title"];
			NSString *subtitle = [eventContext objectForKey:@"subtitle"];
			NSString *notificationType = [eventContext objectForKey:@"notificationType"];
			NSString *activationType = [eventContext objectForKey:@"activationType"];
			
			/* for some reason these two crash (not NSString?) */
			NSString *informativeText = [NSString stringWithString:[eventContext objectForKey:@"informativeText"]];
			NSString *userInfo = [NSString stringWithString:[eventContext objectForKey:@"userInfo"]];
			
			method_id_t methodId = PA_GetMethodID((PA_Unichar *)UN::LISTENER_METHOD.getUTF16StringPtr());
			
			if(methodId)
			{
				PA_Variable	params[6];
				params[0] = PA_CreateVariable(eVK_Unistring);
				params[1] = PA_CreateVariable(eVK_Unistring);
				params[2] = PA_CreateVariable(eVK_Unistring);
				params[3] = PA_CreateVariable(eVK_Unistring);
				params[4] = PA_CreateVariable(eVK_Unistring);
				params[5] = PA_CreateVariable(eVK_Unistring);
				
				UN::setUnistringVariable(&params[0], title);
				UN::setUnistringVariable(&params[1], subtitle);
				UN::setUnistringVariable(&params[2], informativeText);
				UN::setUnistringVariable(&params[3], notificationType);
				UN::setUnistringVariable(&params[4], activationType);
				UN::setUnistringVariable(&params[5], userInfo);
				
				PA_ExecuteMethodByID(methodId, params, 6);
				
				PA_ClearVariable(&params[0]);
				PA_ClearVariable(&params[1]);
				PA_ClearVariable(&params[2]);
				PA_ClearVariable(&params[3]);
				PA_ClearVariable(&params[4]);
				PA_ClearVariable(&params[5]);
				
			}else
			{
				PA_Variable	params[7];
				params[1] = PA_CreateVariable(eVK_Unistring);
				params[2] = PA_CreateVariable(eVK_Unistring);
				params[3] = PA_CreateVariable(eVK_Unistring);
				params[4] = PA_CreateVariable(eVK_Unistring);
				params[5] = PA_CreateVariable(eVK_Unistring);
				params[6] = PA_CreateVariable(eVK_Unistring);
				
				UN::setUnistringVariable(&params[0], title);
				UN::setUnistringVariable(&params[1], subtitle);
				UN::setUnistringVariable(&params[2], informativeText);
				UN::setUnistringVariable(&params[3], notificationType);
				UN::setUnistringVariable(&params[4], activationType);
				UN::setUnistringVariable(&params[5], userInfo);
				
				params[0] = PA_CreateVariable(eVK_Unistring);
				PA_Unistring method = PA_CreateUnistring((PA_Unichar *)UN::LISTENER_METHOD.getUTF16StringPtr());
				PA_SetStringVariable(&params[0], &method);
								
				PA_ExecuteCommandByID(1007, params, 7);
				
				PA_ClearVariable(&params[0]);
				PA_ClearVariable(&params[1]);
				PA_ClearVariable(&params[2]);
				PA_ClearVariable(&params[3]);
				PA_ClearVariable(&params[4]);
				PA_ClearVariable(&params[5]);
				PA_ClearVariable(&params[6]);
				
			}
			
			[UN::listener removeOldestEventContext];
		}
	}
}

#pragma mark -

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
		case kInitPlugin :
		case kServerInitPlugin :
				OnStartup();
				break;
				
		case kCloseProcess :
				OnCloseProcess();
				break;
// --- Notification Center

		case 1 :
			NOTIFICATION_Get_mode(pResult, pParams);
			break;

		case 2 :
			NOTIFICATION_SET_MODE(pResult, pParams);
			break;

		case 3 :
			NOTIFICATION_SET_METHOD(pResult, pParams);
			break;

		case 4 :
			NOTIFICATION_Get_method(pResult, pParams);
			break;

// --- Notification

		case 5 :
			DELIVER_NOTIFICATION(pResult, pParams);
			break;

		case 6 :
			SCHEDULE_NOTIFICATION(pResult, pParams);
			break;

	}
}

#pragma mark -

// ------------------------------ Notification Center -----------------------------

void NOTIFICATION_SET_METHOD(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;

	if(!IsProcessOnExit())
	{
        if(1)
        {
            std::lock_guard<std::mutex> lock(globalMutex2);
            
            UN::LISTENER_METHOD.fromParamAtIndex(pParams, 1);
        }
        PA_RunInMainProcess((PA_RunInMainProcessProcPtr)listener_start, NULL);
        listenerLoopStart();
        returnValue.setIntValue(1);
	}
	
	returnValue.setReturn(pResult);
}

// --------------------------------- Notification ---------------------------------

void SCHEDULE_NOTIFICATION(sLONG_PTR *pResult, PackagePtr pParams)
{
	if(NSFoundationVersionNumber >= NSFoundationVersionNumber10_8)
	{ 
		NSUserNotification *notification = UN::createNotification(pResult, pParams);
		
		C_DATE Param8;
		C_TIME Param9;
		C_TEXT Param10;

		Param8.fromParamAtIndex(pParams, 8);
		Param9.fromParamAtIndex(pParams, 9);
		Param10.fromParamAtIndex(pParams, 10);
				
		NSString *deliveryTimeZone = Param10.copyUTF16String();
		NSTimeZone *timeZone = [NSTimeZone timeZoneWithName:deliveryTimeZone];
		if(!timeZone) timeZone = [NSTimeZone defaultTimeZone];
		
		NSDateComponents *components = [[NSDateComponents alloc]init];
		[components setDay:Param8.getDay()];
		[components setMonth:Param8.getMonth()];
		[components setYear:Param8.getYear()];
		
		[components setSecond:Param9.getSeconds()];
		
		NSCalendar *gregorian = [[NSCalendar alloc]initWithCalendarIdentifier:NSGregorianCalendar];
		notification.deliveryDate = [gregorian dateFromComponents:components];
		[gregorian release];
		[components release];
		
		[[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification:notification];

		[notification release];
	}
}

void NOTIFICATION_Get_method(sLONG_PTR *pResult, PackagePtr pParams)
{
	UN::LISTENER_METHOD.setReturn(pResult);
}

void NOTIFICATION_Get_mode(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	returnValue.setIntValue(UN::shouldPresentNotification);
	returnValue.setReturn(pResult);
}

void DELIVER_NOTIFICATION(sLONG_PTR *pResult, PackagePtr pParams)
{
	if(NSFoundationVersionNumber >= NSFoundationVersionNumber10_8)
	{
		NSUserNotification *notification = UN::createNotification(pResult, pParams);
		
		[[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
		
		[notification release];
	}
}

void NOTIFICATION_SET_MODE(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;
	Param1.fromParamAtIndex(pParams, 1);
    
    if(1)
    {
        std::lock_guard<std::mutex> lock(globalMutex0);
        
        UN::shouldPresentNotification = Param1.getIntValue();
    }
}
