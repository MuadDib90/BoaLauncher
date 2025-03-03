procedure TForm1.previewButtonEnter(Sender: TObject);
var
  pos: Integer;
  control: TControl;
begin
  control:=TControl(TControl(Sender).Parent); //panel pos
  pos:=control.Top;
  dec(pos,control.BorderSpacing.Top);

  if pos < ScrollBox1.VertScrollBar.Position then
    ScrollBox1.VertScrollBar.Position:=pos
  else if pos-ScrollBox1.VertScrollBar.Position > (3 * ScrollBox1.Height div 4) then
    ScrollBox1.VertScrollBar.Position:=pos;
end;

procedure TForm1.addon_panelinfo_click(Sender: TObject);
var
  i,selected: integer;
begin
  if prepareBoA_addons_page_processing then exit;

  selected:=TControl(Sender).Tag;
  addonTitle:=AddonList[selected].Title;
  addonFileName:=AddonList[selected].FileName;

  lblActiveAddon.Caption:='"'+addonTitle+'"';
  chkbLaunchWithAddon.Checked:=true;
  chkbLaunchWithAddonVisibilityChange(true);

  for i:=0 to Length(AddonList)-1 do // colors
  begin
    if i=selected then AddonList[i].Panel.Color:=clActiveCaption
                  else AddonList[i].Panel.Color:=clDefault;
  end;
end;

var
  PreviewForm: TForm;
  PreviewImageFullSize: TImage;
  previewImagesArray: array of TMemoryStream;
  PreviewImageIndex: integer;

procedure TForm1.ClosePreviewClick(Sender: TObject);
begin
  PreviewForm.Close;
end;

procedure TForm1.PreviewKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if (key=37) or (key=38) then
  begin
    PreviewImageIndex:=max(0,PreviewImageIndex-1);
    previewImagesArray[PreviewImageIndex].Position:=0;
    PreviewImageFullSize.Picture.LoadFromStream(previewImagesArray[PreviewImageIndex]);
  end
  else if (key=39) or (key=40) then
  begin
    PreviewImageIndex:=min(Length(previewImagesArray)-1,PreviewImageIndex+1);
    previewImagesArray[PreviewImageIndex].Position:=0;
    PreviewImageFullSize.Picture.LoadFromStream(previewImagesArray[PreviewImageIndex]);
  end
  else
    PreviewForm.Close;
end;

procedure TForm1.PreviewClick(Sender: TObject);
var
  i,j,r: integer;
  MS: TMemoryStream=nil;
begin
  i:=TControl(Sender).Tag;
  if AddonList[i].PrevievImageCount<=0 then exit;

  PreviewImageIndex:=0;

  PreviewForm:=TForm.Create(Form1);
  PreviewForm.Color:=clBlack;
  PreviewForm.BorderStyle:=bsToolWindow;
  PreviewForm.Position:=Form1.Position;
  PreviewForm.Constraints.MaxHeight:=Screen.DesktopHeight * 6 div 10;
  PreviewForm.Constraints.MaxWidth:=Screen.DesktopWidth * 6 div 10;
  PreviewForm.AutoSize:=true;
  PreviewForm.OnClick:=ClosePreviewClick;
  PreviewForm.OnKeyUp:=PreviewKeyUp;

  PreviewImageFullSize:=TImage.Create(PreviewForm);
  PreviewImageFullSize.Parent:=PreviewForm;
  PreviewImageFullSize.Align:=alClient;
  PreviewImageFullSize.BorderSpacing.Around:=10;
  PreviewImageFullSize.Stretch:=true;
  PreviewImageFullSize.StretchInEnabled:=true;
  PreviewImageFullSize.StretchOutEnabled:=true;
  PreviewImageFullSize.OnClick:=ClosePreviewClick;

  //cleanup
  for j:=0 to Length(previewImagesArray)-1 do previewImagesArray[j].free;
  SetLength(previewImagesArray,0);

  //try to load all known existing images
  for j:=1 to AddonList[i].PrevievImageCount do
  begin
    if ExtractSingleFileToStream(AddonList[i].FileName,format('preview/%d.jpg',[j]),MS) then
    begin
      r:=Length(previewImagesArray);
      SetLength(previewImagesArray,r+1);
      previewImagesArray[r]:=MS;
    end;
  end;

  if Length(previewImagesArray)>1 then PreviewForm.Caption:=rsAddonPreview_alt
                                  else PreviewForm.Caption:=rsAddonPreview;

  previewImagesArray[0].Position:=0;
  PreviewImageFullSize.Picture.LoadFromStream(previewImagesArray[0]);

  PreviewForm.ShowModal;

  //cleanup
  PreviewForm.Free;
  for j:=0 to Length(previewImagesArray)-1 do previewImagesArray[j].free;
  SetLength(previewImagesArray,0);
end;

procedure TForm1.preparing_BoA_addons_page;
var
  i: integer;
  MS: TMemoryStream=nil;

  pnlAddon: TPanel;
  btnNameOfAddon: TButton; // user can use tab to select addon
  lblDescOfAddon, lblRequirementsOfAddon: TLabel;
  imgPreview: TImage;

  creditsPnl: TPanel;
  creditsLbl: TLabel;

  progressBar: TProgressBar;

  lastControl: TControl=nil;
const
  leftMargin=10;// pnlAddon margins
  rightMargin=10;
  topMargin=15;
  bottomMargin=15;

  buttonBottomSpace=15;// space between btnNameOfAddon and other control below
  blockSpacing=10; //space between two TextBlocks, or two images
begin
  scanAllBoaFiles(AddonList);

  pnlAddonsContainer.Free;
  pnlAddonsContainer:=TPanel.Create(Form1);
  pnlAddonsContainer.Parent:=ScrollBox1;
  pnlAddonsContainer.Align:=alClient;
  pnlAddonsContainer.BevelOuter:=bvNone;

  progressBar:=TProgressBar.Create(Form1);
  progressBar.Max:=Length(AddonList)-1;
  progressBar.Parent:=Form1;
  progressBar.Anchors:=[];
  progressBar.AnchorParallel(akBottom,0,ScrollBox1);
  progressBar.AnchorParallel(akLeft,0,ScrollBox1);
  progressBar.AnchorParallel(akRight,0,ScrollBox1);

  prepareBoA_addons_page_processing:=true;

  for i:=0 to Length(AddonList)-1 do
  begin
    if terminate then break;

    progressBar.Position:=i;
    Application.ProcessMessages;

    pnlAddon:=TPanel.Create(pnlAddonsContainer);
    pnlAddon.Parent:=pnlAddonsContainer;
    pnlAddon.BorderSpacing.Bottom:=bottomMargin;

    if i=0 then
    begin
      pnlAddon.AnchorParallel(akTop,topMargin,pnlAddonsContainer);
      pnlAddon.AnchorParallel(akLeft,leftMargin,pnlAddonsContainer);
      pnlAddon.AnchorParallel(akRight,rightMargin,pnlAddonsContainer);
    end
    else
      pnlAddon.AnchorToCompanion(akTop,topMargin,lastControl);

    lastControl:=pnlAddon;

    AddonList[i].Panel:=pnlAddon;

    btnNameOfAddon:=TButton.Create(pnlAddonsContainer);
    btnNameOfAddon.Caption:=AddonList[i].Title;
    btnNameOfAddon.Parent:=pnlAddon;
    btnNameOfAddon.AutoSize:=true;
    btnNameOfAddon.AnchorParallel(akTop,topMargin,pnlAddon);
    btnNameOfAddon.AnchorParallel(akLeft,leftMargin,pnlAddon);
    btnNameOfAddon.AnchorParallel(akRight,rightMargin,pnlAddon);

    if AddonList[i].hasAddonInfoTXT then
    begin
      creditsPnl:= TPanel.Create(pnlAddonsContainer);
      creditsPnl.Parent:=pnlAddon;
      creditsPnl.Anchors:=[];
      creditsPnl.AnchorVerticalCenterTo(btnNameOfAddon);
      creditsPnl.AnchorParallel(akRight,5,btnNameOfAddon);
      creditsPnl.BevelOuter:=bvNone;
      creditsPnl.AutoSize:=True;
      creditsLbl:=TLabel.Create(pnlAddonsContainer);
      creditsLbl.Parent:=creditsPnl;
      creditsLbl.Align:=alRight;
      creditsLbl.Font.Style:=[fsBold];
      creditsLbl.BorderSpacing.Bottom:=1;
      creditsLbl.Caption:=Format(rsCreditsAuthor, [AddonList[i].Credits.Replace('&','&&')] );
      if AddonList[i].CreditsFull<>'' then
      begin
        creditsLbl.Caption:=creditsLbl.Caption+'(...)';
        creditsLbl.ShowHint:=true;
        creditsLbl.Hint:=AddonList[i].CreditsFull.Replace('\n',LineEnding);
        creditsLbl.Cursor:=crHandPoint;
      end;

      lblDescOfAddon:=TLabel.Create(pnlAddonsContainer);
      lblDescOfAddon.Parent:=pnlAddon;
      lblDescOfAddon.WordWrap:=true;
      lblDescOfAddon.AutoSize:=true;
      lblDescOfAddon.AnchorToNeighbour(akTop,buttonBottomSpace,btnNameOfAddon);
      lblDescOfAddon.AnchorParallel(akLeft,leftMargin,pnlAddon);
      lblDescOfAddon.AnchorParallel(akRight,rightMargin,pnlAddon);
      lblDescOfAddon.Caption:=rsDescription+LineEnding+AddonList[i].Description;

      lblRequirementsOfAddon:=TLabel.Create(pnlAddonsContainer);
      lblRequirementsOfAddon.Parent:=pnlAddon;
      lblRequirementsOfAddon.WordWrap:=true;
      lblRequirementsOfAddon.AutoSize:=true;
      lblRequirementsOfAddon.AnchorToCompanion(akTop,blockSpacing,lblDescOfAddon);
      lblRequirementsOfAddon.BorderSpacing.Bottom:=bottomMargin;
      lblRequirementsOfAddon.Caption:=rsRequirements+LineEnding+AddonList[i].Requirements;

      //show icon if exists
      if ExtractSingleFileToStream(AddonList[i].FileName,'preview/icon.png',MS)
      then
      begin
        imgPreview:=TImage.Create(pnlAddonsContainer);
        imgPreview.Anchors:=imgPreview.Anchors-[akLeft];
        imgPreview.Picture.LoadFromStream(MS);
        imgPreview.Parent:=pnlAddon;
        imgPreview.Width:=trunc(Screen.PixelsPerInch / Form1.DesignTimePPI * 128);
        imgPreview.Height:=trunc(imgPreview.Picture.Height / imgPreview.Picture.Width * imgPreview.Width);
        imgPreview.AnchorParallel(akRight,0,btnNameOfAddon);
        imgPreview.AnchorToNeighbour(akTop,buttonBottomSpace,btnNameOfAddon);

        lblDescOfAddon.AnchorToNeighbour(akRight,rightMargin,imgPreview);

        imgPreview.Proportional:=true;
        imgPreview.Stretch:=true;

        imgPreview.Tag:=i;
        imgPreview.OnClick:=addon_panelinfo_click;
        if AddonList[i].PrevievImageCount>0 then
        begin
          imgPreview.Cursor:=crHandPoint;
          imgPreview.OnDblClick:=PreviewClick;
          imgPreview.Hint:=rsDoubleClick;
          imgPreview.ShowHint:=true;
        end;
      end
      else if AddonList[i].PrevievImageCount>0 then // else show preview image
      begin
        if ExtractSingleFileToStream(AddonList[i].FileName,'preview/1.jpg',MS) then
        begin
          imgPreview:=TImage.Create(pnlAddonsContainer);
          imgPreview.Anchors:=imgPreview.Anchors-[akLeft];
          imgPreview.Cursor:=crHandPoint;
          imgPreview.Picture.LoadFromStream(MS);
          imgPreview.Parent:=pnlAddon;
          imgPreview.Width:=trunc(Screen.PixelsPerInch / Form1.DesignTimePPI * 128);
          imgPreview.Height:=trunc(imgPreview.Picture.Height / imgPreview.Picture.Width * imgPreview.Width);
          imgPreview.AnchorParallel(akRight,0,btnNameOfAddon);
          imgPreview.AnchorToNeighbour(akTop,buttonBottomSpace,btnNameOfAddon);

          lblDescOfAddon.AnchorToNeighbour(akRight,rightMargin,imgPreview);

          imgPreview.Proportional:=true;
          imgPreview.Stretch:=true;

          imgPreview.Tag:=i;
          imgPreview.OnClick:=addon_panelinfo_click;
          imgPreview.OnDblClick:=PreviewClick;
          imgPreview.Hint:=rsDoubleClick;
          imgPreview.ShowHint:=true;
        end;
      end;

      //make them clickable
      lblDescOfAddon.Tag:=i;
      lblDescOfAddon.OnClick:=addon_panelinfo_click;
      lblRequirementsOfAddon.Tag:=i;
      lblRequirementsOfAddon.OnClick:=addon_panelinfo_click;
    end
    else
    begin
      btnNameOfAddon.BorderSpacing.Bottom:=bottomMargin;
    end;

    //make them clickable (common)
    pnlAddon.Tag:=i;
    pnlAddon.OnClick:=addon_panelinfo_click;
    btnNameOfAddon.Tag:=i;
    btnNameOfAddon.OnClick:=addon_panelinfo_click;
    btnNameOfAddon.OnEnter:=previewButtonEnter;

    pnlAddon.AutoSize:=true;

  end;//end loop: "for i:=0 to fileList.Count-1"

  progressBar.Free;
  if MS<>nil then FreeAndNil(MS);

  prepareBoA_addons_page_processing:=false;
end;
