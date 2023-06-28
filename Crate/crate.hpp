#pragma once
#include <iostream>

#include <deque>
#include <unordered_map>
#include <vector>

#include <source_location>
#include <string_view>
#include <algorithm>

namespace crate
{

	void runTimeLogErr(const std::string_view message,
		const std::source_location location = std::source_location::current())
	{
#ifndef CRATE_DISABLE_LOGGING
		std::cerr << "file: "
			<< location.file_name() << '('
			<< location.line() << ':'
			<< location.column() << ") `"
			<< location.function_name() << "`: "
			<< message << '\n';
#endif
		throw std::runtime_error(message.data());
	}


	struct IAttachment
	{
	
	public:
		bool mDisabled = false;
	};

	static constexpr uint32_t CRATE_MAX_COUNT = 5000;
	static constexpr uint32_t ATTACHMENT_MAX_COUNT = 50;
	static std::deque<uint32_t> unusedIndexes;
	static uint32_t nextID = 0;



	static std::unordered_map<uint32_t, std::unordered_map<uint32_t, IAttachment*>> crateSystem;


	inline [[nodiscard]] bool crateExists(uint32_t* crateID)
	{

		return crateSystem.find(*crateID) != crateSystem.end();
	}

	inline void bind(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (nextID > CRATE_MAX_COUNT)
			runTimeLogErr("Too many crateID:s bound", location);
		if (unusedIndexes.size())
		{
			*crateID = unusedIndexes.front();
			unusedIndexes.pop_front();
	
			crateSystem.insert({*crateID, std::unordered_map<uint32_t, IAttachment*>()});
			return;
		}

		*crateID = nextID;
		crateSystem.insert({ *crateID, std::unordered_map<uint32_t, IAttachment*>() });

		nextID++;
	}

	inline void unbind(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to unbind an unused crateID", location);
		unusedIndexes.push_back(*crateID);
		crateSystem.erase(*crateID);

	}

	uint32_t getAttachmentID()
	{
		static uint32_t attachmentID = 0;
		return attachmentID++;
	}

	template <class Attachment>
	uint32_t getAttachmentID()
	{
		static uint32_t attachmentID = getAttachmentID();
		return attachmentID;
	}

	template <class Attachment>
	inline [[nodiscard]] bool hasAttachment(uint32_t* crateID)
	{
		uint32_t attachmentID = getAttachmentID<Attachment>();
		return crateSystem[*crateID].find(attachmentID) != crateSystem[*crateID].end();
	}

	template <class Attachment, typename... Args>
	inline void bindAttachment(uint32_t* crateID, Args... args)
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to bind an attachment to an unbound crate", std::source_location::current());
		uint32_t attachmentID = getAttachmentID<Attachment>();
		if (hasAttachment<Attachment>(crateID))
			runTimeLogErr("Attachment already bound on crate", std::source_location::current());
		
		Attachment* attachment = new Attachment(args... );
		crateSystem[*crateID][attachmentID] = attachment;
	}

	template <class Attachment>
	inline void unbindAttachment(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to unbind an attachment on an unbound crate", location);
		uint32_t attachmentID = getAttachmentID<Attachment>();
		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Trying to unbind an unbound attachment on crate", location);

		delete crateSystem[*crateID][attachmentID];
		crateSystem[*crateID].erase(attachmentID);
	}

	template <class Attachment>
	inline [[nodiscard]] Attachment* getAttachment(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		uint32_t attachmentID = getAttachmentID<Attachment>();
		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Crate doesn't have the requested attachment", location);

		return (Attachment*)crateSystem[*crateID][attachmentID];
	}

	template <class Attachment>
	inline [[nodiscard]] std::vector<Attachment*> getAllAttachments()
	{
		std::vector<Attachment*> attachments;
		uint32_t attachmentID = getAttachmentID<Attachment>();
		for (auto crate : crateSystem)
		{
			uint32_t crateID = crate.first;
			if (hasAttachment<Attachment>(&crateID))
			{
				if (crate.second[attachmentID]->mDisabled) continue;
				attachments.push_back((Attachment*)crate.second[attachmentID]);
			}
		}
		return attachments;
	}
	
	template <class Attachment, typename Function, typename ...Args>
	inline void updateAttachments(Function function, Args... args)
	{
		const std::vector<Attachment*>& attachments = getAllAttachments<Attachment>();
		std::for_each(attachments.begin(), attachments.end(), [&](Attachment* attachment) {function(attachment, args...); });
	}

	template <class Attachment>
	inline void disableAttachment(uint32_t* crateID, std::source_location location=std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to disable an attachment on an unbound crate", location);
		
		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Trying to disable an unbound attachment", location);

		uint32_t attachmentID = getAttachmentID<Attachment>();
		crateSystem[*crateID][attachmentID]->mDisabled = true;
	}
	
	template <class Attachment>
	inline void enableAttachment(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to enable an attachment on an unbound crate", location);

		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Trying to enable an unbound attachment", location);

		uint32_t attachmentID = getAttachmentID<Attachment>();
		crateSystem[*crateID][attachmentID]->mDisabled = false;
	}
}