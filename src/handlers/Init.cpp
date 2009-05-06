
int stupid_hash(int i)
{
	++i;
	int a = ((i % 11 + 1) * 119);

	if (a == 0)
	{
		return 0;
	}

	return 110905 + (i % 9 + 1) * ((11092004 - i) % a) * 119 + i % 2004;
}

CLIENT_F_FUNC(Init)
{
	if (this->init)
	{
		return false;
	}

	PacketBuilder reply;
	unsigned int challenge;
	unsigned int response;

	reply.SetID(0); // 0 is a special case which sends the connection data

	challenge = reader.GetThree();

	reader.GetChar(); // ?
	reader.GetChar(); // ?
	this->version = reader.GetChar();
	reader.GetChar(); // ?
	reader.GetChar(); // ?
	this->hdid = reader.GetEndString();

	if (the_world->server->AddressBanned(this->GetRemoteAddr()))
	{
		this->Close();
	}

	if (the_world->server->HDIDBanned(this->hdid))
	{
		this->Close();
	}

	/*if (this->version < 27 || this->version > 28)
	{
		// insert "wrong version code" here
		return false;
	}*/
#ifdef DEBUG
	std::printf("Client version: v%i\n", this->version);
#endif // DEBUG

	response = stupid_hash(challenge);

	reply.AddByte(2); // ? (changing breaks EO)
	reply.AddByte(10); // "eID" starting value (1 = +7)
	reply.AddByte(10); // "eID" starting value (1 = +1)
	reply.AddByte(10); // dickwinder multiple
	reply.AddByte(8); // dickwinder multiple
	reply.AddShort(this->id); // player id
	reply.AddThree(response); // hash result

	this->processor.SetEMulti(10,8);

	CLIENT_SENDRAW(reply);

	this->init = true;
	return true;
}
