
// This is converted directly from ASM code in the .27 EO client
// TODO: Clean this shit up.
int stupid_hash(int eax)
{
	int ebx,ecx,edx,esi,edi;

	ebx = 11; // mov ebx,11
	ecx = eax; // mov ecx,[ebp+12]
	esi = 9; // mov esi,9
	++ecx; // inc ecx
	edi = 2004; // mov edi,2004
	eax = ecx; // mov eax,ecx
	edx = 0; // cdq
	edx = eax % ebx; // idiv ebx
	eax = eax / ebx; // (cont.)
	eax = ecx; // mov eax,ecx
	ebx = edx; // mov ebx,edx
	edx = 0; // cdq
	edx = eax % esi; // idiv esi
	eax = eax / esi; // (cont.)
	eax = ecx; // mov eax,ecx
	esi = edx; // mov esi,edx
	edx = 0; // cdq
	edx = eax % edi; // idiv edi
	eax = eax / edi; // (cont.)
	++ebx; // inc ebx
	edi = edx; // mov edi,edx
	edx = ebx; // mov edx,ebx
	eax = 11092004; // mov eax,11092004
	edx <<= 4; // shl edx,4
	eax -= ecx; // sub eax,ecx
	edx -= ebx; // sub edx,ebx
	ecx = eax; // mov ecx,eax
	edx <<= 3; // shl edx,3
	eax = ecx; // mov eax,ecx
	edx -= ebx; // sub edx,ebx
	++esi; // inc esi
	ecx = edx; // mov ecx,edx
	++edi; // inc edi
	edx = 0; // cdq
	edx = eax % ecx; // idiv ecx
	eax = eax / ecx; // (cont.)
	ecx = edx; // mov ecx,edx
	esi *= ecx; // imul esi,ecx
	eax = esi; // mov eax,esi
	eax <<= 4; // shl eax,4
	eax -= esi; // sub eax,esi
	eax <<= 3; // shl eax,3
	eax -= esi; // sub eax,esi
	ecx = eax; // mov ecx,eax
	ecx = ecx + edi + 110904; // lea ecx,[ecx+edi+110904]
	eax = ecx; // mov eax,ecx
	return eax; // ret
}

CLIENT_F_FUNC(Init)
{
	PacketBuilder reply;
	unsigned int challenge;
	unsigned int response;

	if (this->version) return false;

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
