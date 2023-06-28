module;

#include <iostream>

#include <deque>
#include <unordered_map>
#include <vector>

#include <source_location>
#include <string_view>
#include <algorithm>
#include <initializer_list>


export module crate;

namespace crate
{
	export bool CRATE_DISABLE_LOGGING = false;
	export inline uint32_t CRATE_MAX_COUNT = 5000;
	export inline std::deque<uint32_t> unusedIndexes;
	export inline uint32_t nextID = 0;
	

	inline void runTimeLogErr(const std::string_view message,
		const std::source_location location = std::source_location::current())
	{	
		if(!CRATE_DISABLE_LOGGING)
		{
			std::cerr << "file: "
				<< location.file_name() << '('
				<< location.line() << ':'
				<< location.column() << ") `"
				<< location.function_name() << "`: "
				<< message << '\n';
		}
		throw std::runtime_error(message.data());
	}


	export struct IAttachment
	{

	public:
		bool mDisabled = false;
	};

	export inline std::unordered_map<uint32_t, std::unordered_map<uint32_t, IAttachment*>> crateSystem;




	export inline [[nodiscard]] bool crateExists(uint32_t* crateID)
	{

		return crateSystem.find(*crateID) != crateSystem.end();
	}

	export inline void bind(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (nextID > CRATE_MAX_COUNT)
			runTimeLogErr("Too many crateID:s bound", location);
		if (unusedIndexes.size())
		{
			*crateID = unusedIndexes.front();
			unusedIndexes.pop_front();

			crateSystem.insert({ *crateID, std::unordered_map<uint32_t, IAttachment*>() });
			return;
		}

		*crateID = nextID;
		crateSystem.insert({ *crateID, std::unordered_map<uint32_t, IAttachment*>() });

		nextID++;
	}

	export inline void unbind(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to unbind an unused crateID", location);
		unusedIndexes.push_back(*crateID);
		crateSystem.erase(*crateID);
		*crateID = std::numeric_limits<uint32_t>().max();
	}

	inline uint32_t getAttachmentID()
	{
		static uint32_t attachmentID = 0;
		return attachmentID++;
	}

	template <class Attachment>
	inline uint32_t getAttachmentID()
	{
		static uint32_t attachmentID = getAttachmentID();
		return attachmentID;
	}

	export template <class Attachment>
	inline [[nodiscard]] bool hasAttachment(uint32_t* crateID)
	{
		uint32_t attachmentID = getAttachmentID<Attachment>();
		return crateSystem[*crateID].find(attachmentID) != crateSystem[*crateID].end();
	}

	export template <class Attachment, typename... Args>
	inline void bindAttachment(uint32_t* crateID, Args... args)
	{

		if (!crateExists(crateID))
			runTimeLogErr("Trying to bind an attachment to an unbound crate", std::source_location::current());
		uint32_t attachmentID = getAttachmentID<Attachment>();
		if (hasAttachment<Attachment>(crateID))
			runTimeLogErr("Attachment already bound on crate", std::source_location::current());

		Attachment* attachment = new Attachment(args...);
		crateSystem[*crateID][attachmentID] = attachment;
	}

	export template <class Attachment>
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

	export template <class Attachment>
	inline [[nodiscard]] Attachment* getAttachment(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to get an attachment from an unbound crate", location);
		uint32_t attachmentID = getAttachmentID<Attachment>();
		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Crate doesn't have the requested attachment", location);

		return (Attachment*)crateSystem[*crateID][attachmentID];
	}

	export template <class Attachment>
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

	export template <class Attachment, typename Function, typename ...Args>
	inline void updateAttachments(Function function, Args... args)
	{
		const std::vector<Attachment*>& attachments = getAllAttachments<Attachment>();
		std::for_each(attachments.begin(), attachments.end(), [&](Attachment* attachment) {function(attachment, args...); });
	}

	export template <class Attachment>
	inline void disableAttachment(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to disable an attachment on an unbound crate", location);

		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Trying to disable an unbound attachment", location);

		uint32_t attachmentID = getAttachmentID<Attachment>();
		crateSystem[*crateID][attachmentID]->mDisabled = true;
	}

	export template <class Attachment>
	inline void enableAttachment(uint32_t* crateID, std::source_location location = std::source_location::current())
	{
		if (!crateExists(crateID))
			runTimeLogErr("Trying to enable an attachment on an unbound crate", location);

		if (!hasAttachment<Attachment>(crateID))
			runTimeLogErr("Trying to enable an unbound attachment", location);

		uint32_t attachmentID = getAttachmentID<Attachment>();
		crateSystem[*crateID][attachmentID]->mDisabled = false;
	}

	export inline void clearEverything()
	{
		for (const auto& crate : crateSystem)
		{
			uint32_t crateID = crate.first;
			for (const auto& [attachmentID, attachment] : crateSystem[crateID])
			{
				delete attachment;
			}
		}
		crateSystem.clear();
	}
}